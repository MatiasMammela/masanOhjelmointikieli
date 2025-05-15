#include "ir.h"
LLVMModuleRef module;
LLVMContextRef context;
LLVMBuilderRef builder;
symbol_table_t *global_symbols = NULL;

LLVMValueRef ir_make_expression(ast_node_t *n, symbol_table_t *symbols)
{
    switch (n->type)
    {
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        return LLVMConstInt(LLVMInt32TypeInContext(context), n->ast_node_expr_literal_numeric.numeric_value, false);
    case AST_NODE_EXPR_LITERAL_VARIABLE:

        symbol_entry_t *entry = symbol_table_find(symbols, global_symbols, n->ast_node_expr_variable.name);
        if (!entry)
        {
            fprintf(stderr, "Undefined identifier: %s\n", n->ast_node_expr_variable.name);
            exit(1);
        }
        return LLVMBuildLoad2(builder, entry->type, entry->value, n->ast_node_expr_variable.name);
    case AST_NODE_EXPR_BINARY:
        LLVMValueRef left = ir_make_expression(n->ast_node_expr_binary.left, symbols);
        LLVMValueRef right = ir_make_expression(n->ast_node_expr_binary.right, symbols);
        switch (n->ast_node_expr_binary.operation)
        {
        case AST_NODE_OPERATION_ADDITION:
            return LLVMBuildAdd(builder, left, right, "addtmp");
        case AST_NODE_OPERATION_SUBTRACTION:
            return LLVMBuildSub(builder, left, right, "subtmp");
        case AST_NODE_OPERATION_DIVISION:
            return LLVMBuildSDiv(builder, left, right, "divtmp");
        case AST_NODE_OPERATION_MULTIPLICATION:
            return LLVMBuildMul(builder, left, right, "multmp");
        default:
            fprintf(stderr, "Unsupported binary operator\n");
            exit(1);
        }
    case AST_NODE_EXPR_UNARY:
    {
        LLVMValueRef operand = ir_make_expression(n->ast_node_expr_unary.value, symbols);
        switch (n->ast_node_expr_unary.operation)
        {
        case AST_NODE_OPERATION_SUBTRACTION:
            // Negate the operand: -x
            return LLVMBuildNeg(builder, operand, "negtmp");
        case AST_NODE_OPERATION_ADDITION:
            return operand;
        default:
            fprintf(stderr, "Unsupported unary operator\n");
            exit(1);
        }
    }
    default:
        fprintf(stderr, "Unsupported AST expression node\n");
        exit(1);
    }
}
LLVMTypeRef *ir_get_parameter_types(ast_node_t **n, size_t count)
{
    LLVMTypeRef *param_types = malloc(sizeof(LLVMTypeRef) * count);
    for (size_t i = 0; i < count; i++)
    {
        param_types[i] = ir_make_type(n[i]->ast_node_parameter.type->kind);
    }
    return param_types;
}

void ir_store_parameter_values(ast_node_t **n, size_t count, LLVMValueRef func, symbol_table_t *symbols)
{
    for (size_t i = 0; i < count; i++)
    {
        LLVMValueRef param_value = LLVMGetParam(func, i);
        LLVMSetValueName(param_value, n[i]->ast_node_parameter.name);
        LLVMTypeRef param_type = LLVMTypeOf(param_value);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, param_type, n[i]->ast_node_parameter.name);
        LLVMBuildStore(builder, param_value, alloca);
        symbol_entry_t entry = {
            .name = n[i]->ast_node_parameter.name,
            .value = alloca,
            .type = param_type};
        symbol_table_add(symbols, entry);
    }
}
LLVMValueRef ir_make_assignment(ast_node_t *n, symbol_table_t *symbols)
{
    const char *name = n->ast_node_assignment.name;
    LLVMTypeRef type = ir_make_type(n->ast_node_assignment.type->kind);

    LLVMValueRef ptr = LLVMBuildAlloca(builder, type, name);

    symbol_entry_t entry = {
        .name = name,
        .value = ptr,
        .type = type,
    };
    symbol_table_add(symbols, entry);

    LLVMValueRef value = NULL;
    if (n->ast_node_assignment.expr != NULL)
    {
        if (n->ast_node_assignment.expr->type == AST_NODE_FUNCTION_CALL)
        {
            printf("Its a func call\n");
            LLVMValueRef call_val = ir_make_function_call(n->ast_node_assignment.expr, symbols);

            LLVMTypeRef call_type = LLVMTypeOf(call_val);
            if (LLVMGetTypeKind(call_type) != LLVMVoidTypeKind)
            {
                value = call_val;
                LLVMBuildStore(builder, value, ptr);
            }
            else
            {
                value = NULL;
            }
        }
        else
        {
            value = ir_make_expression(n->ast_node_assignment.expr, symbols);
            LLVMBuildStore(builder, value, ptr);
        }
    }
    else
    {
        value = LLVMConstNull(type);
        LLVMBuildStore(builder, value, ptr);
    }

    LLVMBuildStore(builder, value, ptr);

    return ptr;
}

LLVMValueRef ir_make_reassignment(ast_node_t *n, symbol_table_t *symbols)
{
    symbol_entry_t *symbol = symbol_table_find(symbols, global_symbols, n->ast_node_reassignment.name);
    LLVMValueRef ptr = symbol->value;
    if (!ptr)
    {
        fprintf(stderr, "Error: variable %s not declared\n", n->ast_node_reassignment.name);
        exit(1);
    }

    LLVMValueRef value = ir_make_expression(n->ast_node_reassignment.expr, symbols);
    LLVMBuildStore(builder, value, ptr);
    return value;
}

LLVMValueRef ir_make_return(ast_node_t *n, symbol_table_t *symbols)
{
    LLVMValueRef return_value;
    switch (n->ast_node_return.value->type)
    {
    case AST_NODE_EXPR_LITERAL_NUMERIC:
    case AST_NODE_EXPR_LITERAL_VARIABLE:
    case AST_NODE_EXPR_BINARY:
    case AST_NODE_EXPR_UNARY:
        return_value = ir_make_expression(n->ast_node_return.value, symbols);
        break;
    case AST_NODE_FUNCTION_CALL:
        break;
    default:
        fprintf(stderr, "Error: return type %d not found\n", n->ast_node_return.value->type);
        break;
    }
    LLVMBuildRet(builder, return_value);
}

LLVMValueRef ir_make_function_call(ast_node_t *n, symbol_table_t *symbols)
{
    const char *func_name = n->ast_node_function_call.name;
    LLVMValueRef func = LLVMGetNamedFunction(module, func_name);
    if (!func)
    {
        fprintf(stderr, "Function not found: %s\n", func_name);
        exit(1);
    }

    size_t count = n->ast_node_function_call.param_count;
    LLVMValueRef *args = malloc(sizeof(LLVMValueRef) * count);
    for (size_t i = 0; i < count; i++)
    {
        args[i] = ir_make_expression(n->ast_node_function_call.params[i], symbols);
    }
    LLVMValueRef call = LLVMBuildCall2(
        builder,
        LLVMGetElementType(LLVMTypeOf(func)),
        func,
        args,
        count,
        "");

    free(args);
    return call;
}

LLVMValueRef *ir_make_stmt_block(ast_node_t *n, symbol_table_t *symbols)
{
    ast_node_t *stmt = n->ast_node_stmt_block.statements.first;
    for (size_t i = 0; n->ast_node_stmt_block.statements.count > i; i++)
    {
        switch (stmt->type)
        {
        case AST_NODE_TYPE_ASSIGNMENT:
            ir_make_assignment(stmt, symbols);
            break;
        case AST_NODE_TYPE_REASSIGNMENT:
            ir_make_reassignment(stmt, symbols);
            break;
        case AST_NODE_TYPE_STMT_RETURN:
            ir_make_return(stmt, symbols);
            break;
        case AST_NODE_FUNCTION_CALL:
            ir_make_function_call(stmt, symbols);
            break;
        default:
            break;
        }
        stmt = stmt->next;
    }
}
void ir_make_tlc_function_declaration(ast_node_t *n)
{
    symbol_table_t *symbols = symbol_table_create();

    LLVMTypeRef *param_types = ir_get_parameter_types(
        n->ast_node_tlc_function_declaration.params,
        n->ast_node_tlc_function_declaration.param_count);

    LLVMTypeRef func_type = LLVMFunctionType(
        ir_make_type(n->ast_node_tlc_function_declaration.return_type->kind),
        param_types,
        n->ast_node_tlc_function_declaration.param_count,
        0);

    LLVMValueRef func = LLVMAddFunction(
        module,
        n->ast_node_tlc_function_declaration.name,
        func_type);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(
        context,
        func,
        "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    ir_store_parameter_values(
        n->ast_node_tlc_function_declaration.params,
        n->ast_node_tlc_function_declaration.param_count,
        func,
        symbols);

    symbol_print_table(symbols, global_symbols);
    ir_make_stmt_block(n->ast_node_tlc_function_declaration.statements, symbols);
}

LLVMTypeRef ir_make_type(ast_node_type_kind_t t)
{
    switch (t)
    {
    case TYPE_INT:
        return LLVMInt32TypeInContext(context);
    case TYPE_VOID:
        return LLVMVoidType();
    default:
        fprintf(stderr, "Unknown type kind: %d\n", t);
        exit(1);
        break;
    }
}

void ir_make_tlc_declaration(ast_node_t *n)
{
    const char *name = n->ast_node_tlc_declaration.name;
    LLVMTypeRef type = ir_make_type(n->ast_node_tlc_declaration.type->kind);
    LLVMValueRef global = LLVMAddGlobal(module, type, name);

    symbol_entry_t entry = {
        .name = name,
        .value = global,
        .type = type,
    };
    symbol_table_add(global_symbols, entry);

    LLVMValueRef value = NULL;
    if (n->ast_node_tlc_declaration.expr != NULL)
    {
        value = ir_make_expression(n->ast_node_tlc_declaration.expr, global_symbols);
    }

    if (value != NULL && LLVMIsConstant(value))
    {
        LLVMSetInitializer(global, value);
    }
    else
    {
        LLVMSetInitializer(global, LLVMConstNull(type));
    }
}

void ir_init()
{
    printf("\n\nIR INIT\n\n");
    context = LLVMContextCreate();
    module = LLVMModuleCreateWithNameInContext("my_module", context);
    builder = LLVMCreateBuilderInContext(context);
    global_symbols = symbol_table_create();
    ast_node_t *tlc_node = head;
    while (tlc_node != NULL)
    {
        switch (tlc_node->type)
        {
        case AST_NODE_TLC_DECLARATION:
            ir_make_tlc_declaration(tlc_node);
            break;
        case AST_NODE_TLC_FUNCTION_DECLARATION:
            ir_make_tlc_function_declaration(tlc_node);
            break;
        default:
            break;
        }
        tlc_node = tlc_node->next;
    }

    char *ir_str = LLVMPrintModuleToString(module);
    printf("%s\n", ir_str);

    LLVMDisposeMessage(ir_str);
    LLVMPrintModuleToFile(module, "resources/output.ll", NULL);
    LLVMDisposeBuilder(builder);
    LLVMDisposeModule(module);
    LLVMContextDispose(context);
}