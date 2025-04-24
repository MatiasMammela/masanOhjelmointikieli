#include "ir.h"

LLVMModuleRef module;
LLVMContextRef context;
LLVMBuilderRef builder;
symbol_table_t *global_symbols = NULL;

int is_compile_time_constant(LLVMValueRef value)
{
    return LLVMIsAConstantInt(value) != NULL;
}

intmax_t extract_constant_int_value(LLVMValueRef value)
{
    if (LLVMIsAConstantInt(value))
    {
        return LLVMConstIntGetSExtValue(value);
    }

    if (LLVMIsAGlobalVariable(value))
    {
        LLVMValueRef initializer = LLVMGetInitializer(value);
        if (initializer && LLVMIsAConstantInt(initializer))
        {
            return LLVMConstIntGetSExtValue(initializer);
        }
    }

    printf("Cannot extract compile-time constant from this LLVM value\n");
    return 0;
}

intmax_t evaluate(ast_node_t *node, symbol_table_t *symbols)
{
    if (!node)
    {
        return 0;
    }

    switch (node->type)
    {
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        return (intmax_t)node->ast_node_expr_literal_numeric.numeric_value;
    case AST_NODE_EXPR_UNARY:
    {
        intmax_t val = evaluate(node->ast_node_expr_unary.value, symbols);
        switch (node->ast_node_expr_unary.operation)
        {
        case AST_NODE_OPERATION_ADDITION:
            return val;
        case AST_NODE_OPERATION_SUBTRACTION:
            return -val;
        default:
            printf("Unknown unary operation\n");
            return 0;
        }
    }
    case AST_NODE_EXPR_LITERAL_VARIABLE:
        symbol_entry_t *e = symbol_table_find(symbols, node->ast_node_expr_variable.name);
        intmax_t val = (intmax_t)extract_constant_int_value(e->value);
        return val;
    case AST_NODE_EXPR_BINARY:
    {
        intmax_t left_value = evaluate(node->ast_node_expr_binary.left, symbols);
        intmax_t right_value = evaluate(node->ast_node_expr_binary.right, symbols);

        switch (node->ast_node_expr_binary.operation)
        {
        case AST_NODE_OPERATION_ADDITION:
            return left_value + right_value;
        case AST_NODE_OPERATION_SUBTRACTION:
            return left_value - right_value;
        case AST_NODE_OPERATION_MULTIPLICATION:
            return left_value * right_value;
        case AST_NODE_OPERATION_DIVISION:
            if (right_value != 0)
            {
                return left_value / right_value;
            }
            else
            {
                printf("Division by zero error\n");
                return 0;
            }
        default:
            printf("Unknown operation\n");
            return 0;
        }
    }
    default:
        printf("Unsupported node type\n");
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
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMSetValueName(param, n[i]->ast_node_parameter.name);

        symbol_entry_t entry = {
            .name = n[i]->ast_node_parameter.name,
            .value = param,
        };
        symbol_table_add(symbols, entry);
    }
}

LLVMTypeRef ir_make_type(ast_node_type_kind_t t)
{
    switch (t)
    {
    case TYPE_INT:
        return LLVMInt32Type();
    case TYPE_VOID:
        return LLVMVoidType();
    default:
        break;
    }
}

LLVMValueRef ir_make_assignment(ast_node_t *n, symbol_table_t *symbols)
{
    const char *name = n->ast_node_assignment.name;
    LLVMTypeRef type = ir_make_type(n->ast_node_assignment.type->kind);

    LLVMValueRef ptr = LLVMBuildAlloca(builder, type, name);
    LLVMValueRef value = NULL;
    if (n->ast_node_assignment.expr != NULL)
    {
        value = ir_make_expression(n->ast_node_assignment.expr, symbols);
    }
    else
    {
        value = LLVMConstNull(type);
    }
    if (!value)
    {
        printf("Value is null\n");
        exit(1);
    }
    LLVMBuildStore(builder, value, ptr);
    symbol_entry_t entry = {
        .name = name,
        .value = ptr,
    };
    symbol_table_add(symbols, entry);

    return value;
}

LLVMValueRef ir_make_int_type()
{
}

LLVMValueRef ir_make_return(ast_node_t *n, symbol_table_t *symbols)
{
    LLVMValueRef return_value = NULL;
    switch (n->ast_node_return.value->type)
    {
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        intmax_t numeric_value = n->ast_node_return.value->ast_node_expr_literal_numeric.numeric_value;
        LLVMTypeRef return_type = LLVMInt64TypeInContext(context);
        printf("Returning numeric value: %jd\n", numeric_value);
        return_value = LLVMConstInt(return_type, numeric_value, 0);
        break;
    case AST_NODE_EXPR_LITERAL_VARIABLE:
    {
        const char *var_name = n->ast_node_return.value->ast_node_expr_variable.name;
        symbol_entry_t *entry = symbol_table_find(symbols, var_name);
        if (entry != NULL)
        {
            LLVMTypeRef type = LLVMTypeOf(entry->value);
            if (LLVMGetTypeKind(type) == LLVMPointerTypeKind)
            {
                return_value = LLVMBuildLoad2(builder, LLVMGetElementType(type), entry->value, "load_ret");
            }
            else
            {
                return_value = entry->value;
            }
        }
        else
        {
            printf("Error: Variable %s not found in symbol table\n", var_name);
            return NULL;
        }
        break;
    }
    default:
        break;
    }

    if (return_value != NULL)
    {
        LLVMBuildRet(builder, return_value);
    }

    return return_value;
}

LLVMValueRef ir_make_reassignment(ast_node_t *n, symbol_table_t *symbols)
{
    symbol_entry_t *symbol = symbol_table_find(symbols, n->ast_node_reassignment.name);
    LLVMValueRef ptr = symbol->value;
    if (!ptr)
    {
        fprintf(stderr, "Error: variable '%s' not declared\n", n->ast_node_reassignment.name);
        exit(1);
    }

    LLVMValueRef value = ir_make_expression(n->ast_node_reassignment.expr, symbols);
    LLVMBuildStore(builder, value, ptr);
    return value;
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
            break;
        case AST_NODE_TYPE_STMT_RETURN:
            ir_make_return(stmt, symbols);
            break;
        default:
            break;
        }
        stmt = stmt->next;
    }
}
LLVMValueRef ir_make_function_declaration(ast_node_t *n)
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

    ir_store_parameter_values(
        n->ast_node_tlc_function_declaration.params,
        n->ast_node_tlc_function_declaration.param_count,
        func,
        symbols);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(
        context,
        func,
        "entry");

    LLVMPositionBuilderAtEnd(builder, entry);
    ir_make_stmt_block(n->ast_node_tlc_function_declaration.statements, symbols);

    return func;
}

LLVMValueRef ir_make_expression(ast_node_t *n, symbol_table_t *symbols)
{
    switch (n->type)
    {
    case AST_NODE_EXPR_BINARY:
    case AST_NODE_EXPR_UNARY:
    case AST_NODE_TYPE_ASSIGNMENT:
    case AST_NODE_TYPE_REASSIGNMENT:
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        return ir_evaluate_expression(n, symbols);
    default:

        printf("No case in ir_make_expression. Tried %d\n", n->type);
        exit(1);
        break;
    }
}

LLVMValueRef ir_evaluate_expression(ast_node_t *start, symbol_table_t *symbols)
{
    intmax_t result = evaluate(start, symbols);
    return LLVMConstInt(LLVMInt32TypeInContext(context), result, 1);
}

LLVMValueRef ir_make_tlc_declaration(ast_node_t *n)
{
    const char *name = n->ast_node_tlc_declaration.name;
    LLVMTypeRef type = ir_make_type(n->ast_node_tlc_declaration.type->kind);
    LLVMValueRef global = LLVMAddGlobal(module, type, name);

    symbol_entry_t entry = {
        .name = name,
        .value = global,
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
    return global;
}

void ir_init()
{

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
            ir_make_function_declaration(tlc_node);
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

// LLVMModuleRef module = LLVMModuleCreateWithName("module");
// LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
// LLVMValueRef func = LLVMAddFunction(module, "main", func_type);
// LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
// LLVMBuilderRef builder = LLVMCreateBuilder();
// LLVMPositionBuilderAtEnd(builder, entry);
// LLVMValueRef ret = LLVMConstInt(LLVMInt32Type(), 69, 0);
// LLVMBuildRet(builder, ret);
// LLVMPrintModuleToFile(module, "output.ll", NULL);