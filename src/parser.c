#include "parser.h"
ast_node_t *nodes;
ast_node_t *head = NULL;
static ast_node_t *tail = NULL;

// MAKE
ast_node_t *make_ast_node_function_call(const char *name, ast_node_t **params, size_t param_count)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_FUNCTION_CALL;
    n->ast_node_function_call.name = name;
    n->ast_node_function_call.params = params;
    n->ast_node_function_call.param_count = param_count;
    return n;
}

ast_node_t *make_ast_node_expr_literal_numeric(intmax_t value)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_LITERAL_NUMERIC;
    n->ast_node_expr_literal_numeric.numeric_value = value;
    return n;
}

ast_node_t *make_ast_node_expr_binary(ast_node_t *left, ast_node_operation_t operation, ast_node_t *right)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_BINARY;
    n->ast_node_expr_binary.left = left;
    n->ast_node_expr_binary.operation = operation;
    n->ast_node_expr_binary.right = right;
    return n;
}

ast_node_t *make_ast_node_assignment(ast_type_t *type, const char *name, ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_ASSIGNMENT;
    n->ast_node_assignment.type = type;
    n->ast_node_assignment.name = name;
    n->ast_node_assignment.expr = expr;
    return n;
}

ast_node_t *make_ast_node_tlc_declaration(ast_type_t *type, const char *name, ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TLC_DECLARATION;
    n->ast_node_tlc_declaration.type = type;
    n->ast_node_tlc_declaration.name = name;
    n->ast_node_tlc_declaration.expr = expr;
    return n;
}

ast_node_t *make_ast_node_expr_literal_variable(const char *name)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_LITERAL_VARIABLE;
    n->ast_node_expr_variable.name = name;
    return n;
}
ast_node_t *make_ast_node_reassignment(const char *name, ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_REASSIGNMENT;
    n->ast_node_reassignment.name = name;
    n->ast_node_reassignment.expr = expr;
    return n;
}
ast_node_t *make_ast_node_expr_unary(ast_node_operation_t operation, ast_node_t *value)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_UNARY;
    n->ast_node_expr_unary.operation = operation;
    n->ast_node_expr_unary.value = value;
    return n;
}
ast_node_t *make_ast_node_stmt_expression(ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_STMT_EXPRESSION;
    n->ast_node_tlc_declaration.expr = expr;
    return n;
}
ast_node_t *make_ast_node_tlc_function_declaration(ast_type_t *return_type, const char *name, ast_node_t **params, size_t param_count, ast_node_t *statements)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TLC_FUNCTION_DECLARATION;
    n->ast_node_tlc_function_declaration.return_type = return_type;
    n->ast_node_tlc_function_declaration.name = name;
    n->ast_node_tlc_function_declaration.params = params;
    n->ast_node_tlc_function_declaration.param_count = param_count;
    n->ast_node_tlc_function_declaration.statements = statements;
    return n;
}
ast_node_t *make_ast_node_return(ast_node_t *value)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_STMT_RETURN;
    n->ast_node_return.value = value;
    return n;
}
ast_node_t *make_ast_node_stmt_block(ast_node_list_t stmt_list)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_STMT_BLOCK;
    n->ast_node_stmt_block.statements = stmt_list;
    return n;
}

// MISC
__attribute__((noreturn)) void panic(const char *err, const char *tokenval, uint32_t lnum)
{
    printf("Parsing failed: %s %s at %d \n", err, tokenval, lnum);
    exit(1);
    __builtin_unreachable();
}
ast_node_list_t *ast_node_list_create()
{
    ast_node_list_t *l = malloc(sizeof(ast_node_list_t));
    l->count = 0;
    l->first = NULL;
    l->last = NULL;
    return l;
}
void add_ast_node(ast_node_t *n)
{
    n->next = NULL;
    if (tail)
    {
        tail->next = n;
    }
    else
    {
        head = n;
    }
    tail = n;
}
void ast_node_list_append(ast_node_list_t *list, ast_node_t *node)
{
    if (list->first == NULL)
    {
        list->first = node;
    }
    else if (list->last)
    {
        list->last->next = node;
    }
    list->last = node;
    list->count++;
}
ast_node_t *parse_ast_node_return()
{
    token_t *return_token = advance_token();
    ast_node_t *expression = parse_ast_node_stmt_expression();
    return make_ast_node_return(expression);
}
ast_type_t *parse_ast_type(token_t *t)
{
    ast_type_t *type = malloc(sizeof(ast_type_t));
    if (strcmp(t->value, "i32") == 0)
    {
        type->kind = TYPE_INT;
        type->integer.is_signed = true;
        type->integer.bit_size = 32;
    }
    else if (strcmp(t->value, "void") == 0)
    {
        type->kind = TYPE_VOID;
    }
    else
    {
        panic("Unknown type at ", t->value, t->linenumber);
    }
    return type;
}

ast_node_t *parse_ast_node_tlc_function_declaration()
{
    token_t *token_fn = advance_assert_token_type(TOKEN_FUNCTION);
    token_t *token_type = advance_type();
    token_t *token_name = advance_token();
    token_t *token = advance_token();

    // Parse parameters
    ast_node_t **params = NULL;
    size_t param_count = 0;
    size_t param_capacity = 0;
    if (token->next->type != TOKEN_RPAREN)
    {
        printf("Params\n");
        while (token->type != TOKEN_RPAREN)
        {
            if (param_count == param_capacity)
            {
                // if 0 set to 4 else double
                param_capacity = (param_capacity == 0) ? 4 : param_capacity * 2;
                ast_node_t **new_params = realloc(params, sizeof(ast_node_t *) * param_capacity);
                if (!new_params)
                {
                    perror("Failed to realloc for function parameters!!\n");
                    exit(1);
                }
                params = new_params;
            }

            ast_node_t *param = malloc(sizeof(ast_node_t));
            param->type = AST_NODE_PARAMETER;
            token_t *type = advance_type();
            token_t *name = advance_token();
            param->ast_node_parameter.type = parse_ast_type(type);
            param->ast_node_parameter.name = name->value;
            params[param_count] = param;
            token_t *comma = advance_token();
            param_count++;
            if (comma->type != TOKEN_COMMA)
            {
                break;
            }
        }
    }
    else
    {
        advance_assert_token_type(TOKEN_RPAREN);
    }

    // Parse function block
    ast_node_t *statements = parse_ast_node_stmt_block();
    printf("Param count %d ", param_count);

    for (int i = 0; i < param_count; i++)
    {
        printf("Param  %d %s \n", params[i]->ast_node_parameter.type->kind, params[i]->ast_node_parameter.name);
    }

    return make_ast_node_tlc_function_declaration(
        parse_ast_type(token_type),
        token_name->value,
        params,
        param_count,
        statements);
}

ast_node_operation_t get_operation_type(token_t *t)
{
    switch (t->type)
    {
    case TOKEN_PLUS:
        return AST_NODE_OPERATION_ADDITION;
        break;
    case TOKEN_MINUS:
        return AST_NODE_OPERATION_SUBTRACTION;
        break;
    case TOKEN_STAR:
        return AST_NODE_OPERATION_MULTIPLICATION;
        break;
    case TOKEN_SLASH:
        return AST_NODE_OPERATION_DIVISION;
        break;
    default:
        panic("Uknown token type %s at %d", t->value, t->linenumber);
        break;
    }
}
int get_binding_power(token_type_t type)
{
    switch (type)
    {
    case TOKEN_STAR:
    case TOKEN_SLASH:
        return 20;
        break;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 10;
    default:
        return 0;
        break;
    }
}

// PARSE
ast_node_t *parse_ast_node_function_call_params()
{
    token_t *left_token = peek_token();
    ast_node_t *left_node;
    switch (left_token->type)
    {
    case TOKEN_IDENTIFIER:
        if (left_token->next->type == TOKEN_LPAREN)
        {
            left_node = parse_ast_node_function_call();
            break;
        }
        else
        {
            left_node = parse_ast_node_expr_literal_variable();
        }
        break;
    case TOKEN_NUMBER:
        left_node = parse_ast_node_expr_literal_numeric();
        break;
    case TOKEN_LPAREN:
        left_node = parse_ast_node_expr_primary();
        break;
    case TOKEN_MINUS:
    case TOKEN_PLUS:
        left_node = parse_ast_node_expr_unary();
        break;
    default:
        break;
    }

    token_t *left_next = peek_token();
    switch (left_next->type)
    {
    case TOKEN_COMMA:
        return left_node;
        break;
    default:
        ast_node_t *expr = parse_ast_node_expr_binary(left_node, 0);
        return expr;
        break;
    }
}

ast_node_t *parse_ast_node_function_call()
{

    printf("Parsing a function call\n");
    token_t *variable = advance_token();
    ast_node_t **params = NULL;
    size_t param_count = 0;
    size_t param_capacity = 0;
    token_t *token = advance_token();
    if (token->next->type != TOKEN_RPAREN)
    {
        while (token->type != TOKEN_RPAREN)
        {
            if (param_count == param_capacity)
            {
                // if 0 set to 4 else double
                param_capacity = (param_capacity == 0) ? 4 : param_capacity * 2;
                ast_node_t **new_params = realloc(params, sizeof(ast_node_t *) * param_capacity);
                if (!new_params)
                {
                    perror("Failed to realloc for function parameters!!\n");
                    exit(1);
                }
                params = new_params;
            }
            ast_node_t *param = malloc(sizeof(ast_node_t));
            param = parse_ast_node_function_call_params();
            params[param_count] = param;
            token_t *comma = peek_token();
            param_count++;
            if (comma->type != TOKEN_COMMA)
            {
                break;
            }
            advance_assert_token_type(TOKEN_COMMA);
        }
    }

    advance_assert_token_type(TOKEN_RPAREN);

    return make_ast_node_function_call(variable->value, params, param_count);
}
ast_node_t *parse_ast_node_stmt_block()
{
    printf("Starting to parse a stmt block \n");
    token_t *lbracket = advance_assert_token_type(TOKEN_LBRACKET);
    token_t *token = peek_token();
    ast_node_list_t *statements = ast_node_list_create();
    while (token->type != TOKEN_RBRACKET)
    {
        switch (token->type)
        {
        case TOKEN_TYPE:
            ast_node_t *decl = parse_ast_node_assignment();
            ast_node_list_append(statements, decl);
            break;
        case TOKEN_IDENTIFIER:

            if (token->next->type == TOKEN_LPAREN)
            {
                ast_node_t *statement = parse_ast_node_function_call();
                ast_node_list_append(statements, statement);
                advance_assert_token_type(TOKEN_SEMICOLON);
                break;
            }
            else
            {
                ast_node_t *statement = parse_ast_node_reassignment();
                ast_node_list_append(statements, statement);
            }
            break;
        case TOKEN_RETURN:
            ast_node_t *return_node = parse_ast_node_return();
            ast_node_list_append(statements, return_node);
            break;
        default:
            panic("Undefined token at parse_ast_node_stmt_block", token->value, token->linenumber);
            break;
        }
        token = peek_token();
    }
    advance_assert_token_type(TOKEN_RBRACKET);
    return make_ast_node_stmt_block(*statements);
}

ast_node_t *parse_ast_node_expr_literal_variable()
{
    token_t *name = advance_token();
    return make_ast_node_expr_literal_variable(name->value);
}

ast_node_t *parse_ast_node_expr_primary()
{
    token_t *tok = peek_token();

    switch (tok->type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    {
        return parse_ast_node_expr_unary();
    }
    case TOKEN_LPAREN:
    {
        advance_token(); // (
        ast_node_t *expr = parse_ast_node_expr_binary(parse_ast_node_expr_primary(), 0);

        advance_token();
        return expr;
    }
    case TOKEN_IDENTIFIER:
        return parse_ast_node_expr_literal_variable();
    case TOKEN_NUMBER:
        return parse_ast_node_expr_literal_numeric();
    default:
        printf("Unexpected token in primary expression: %d\n", tok->type);
        exit(1);
    }
}
ast_node_t *parse_ast_node_expr_binary(ast_node_t *left, int min_binding_power)
{
    while (true)
    {
        token_t *operation = peek_token();
        int binding_power = get_binding_power(operation->type);
        if (binding_power <= min_binding_power)
        {
            break;
        }

        advance_token();

        // Get the right
        token_t *right_token = peek_token();
        ast_node_t *right_node;
        switch (right_token->type)
        {
        case TOKEN_LPAREN:
            right_node = parse_ast_node_expr_primary();
            break;
        case TOKEN_IDENTIFIER:
            right_node = parse_ast_node_expr_literal_variable();
            break;
        case TOKEN_NUMBER:
            right_node = parse_ast_node_expr_literal_numeric();
        default:
            break;
        }

        // Peek the next operation to determine if it has higher binding power
        token_t *next_operation = peek_token();
        // Get the binding power of the next operation
        int next_binding_power = get_binding_power(next_operation->type);
        // if the next binding power is larger than the first one, parse it before combining
        if (binding_power < next_binding_power)
        {
            right_node = parse_ast_node_expr_binary(right_node, binding_power);
        }

        // Create the node
        ast_node_operation_t operation_type = get_operation_type(operation);
        left = make_ast_node_expr_binary(left, operation_type, right_node);
    }
    return left;
}

ast_node_t *parse_ast_node_stmt_expression()
{
    token_t *left_token = peek_token();
    ast_node_t *left_node;
    switch (left_token->type)
    {
    case TOKEN_IDENTIFIER:
        if (left_token->next->type == TOKEN_LPAREN)
        {
            left_node = parse_ast_node_function_call();
            break;
        }
        else
        {
            left_node = parse_ast_node_expr_literal_variable();
        }
        break;
    case TOKEN_NUMBER:
        left_node = parse_ast_node_expr_literal_numeric();
        break;
    case TOKEN_LPAREN:
        left_node = parse_ast_node_expr_primary();
        break;
    case TOKEN_MINUS:
    case TOKEN_PLUS:
        left_node = parse_ast_node_expr_unary();
        break;
    default:
        break;
    }

    token_t *left_next = peek_token();
    switch (left_next->type)
    {
    case TOKEN_SEMICOLON:
        advance_assert_token_type(TOKEN_SEMICOLON);
        return left_node;
        break;

    default:
        ast_node_t *expr = parse_ast_node_expr_binary(left_node, 0);
        advance_assert_token_type(TOKEN_SEMICOLON);
        return expr;
        break;
    }
}
ast_node_t *parse_ast_node_expr_unary()
{
    token_t *operator = advance_operation();
    ast_node_operation_t operation_type = get_operation_type(operator);
    token_t *token_value = peek_token();
    ast_node_t *node_value;
    switch (token_value->type)
    {
    case TOKEN_NUMBER:
        node_value = parse_ast_node_expr_literal_numeric();
        break;
    case TOKEN_IDENTIFIER:
        node_value = parse_ast_node_expr_literal_variable();
        break;
    default:
        panic("Unexpected token at unary operation %s at %d ", token_value->value, token_value->linenumber);
        break;
    }
    return make_ast_node_expr_unary(operation_type, node_value);
}

ast_node_t *parse_ast_node_reassignment()
{

    token_t *variable = advance_token();
    advance_token();
    ast_node_t *expression = parse_ast_node_stmt_expression();
    return make_ast_node_reassignment(
        variable->value,
        expression);
}

ast_node_t *parse_ast_node_assignment()
{
    token_t *type_token = advance_type();
    token_t *name_token = advance_token();
    token_t *equal = advance_token();
    if (equal->type == TOKEN_SEMICOLON)
    {
        return make_ast_node_assignment(
            parse_ast_type(type_token),
            name_token->value,
            NULL);
    }
    ast_node_t *expression = parse_ast_node_stmt_expression();
    return make_ast_node_assignment(
        parse_ast_type(type_token),
        name_token->value,
        expression);
}

ast_node_t *parse_ast_node_tlc_declaration()
{

    token_t *type_token = advance_type();
    token_t *name_token = advance_token();
    token_t *equal = advance_token();

    // If the decl doesnt have a statement
    if (equal->type == TOKEN_SEMICOLON)
    {
        return make_ast_node_tlc_declaration(
            parse_ast_type(type_token),
            name_token->value,
            NULL);
    }

    ast_node_t *expression = parse_ast_node_stmt_expression();
    return make_ast_node_tlc_declaration(
        parse_ast_type(type_token),
        name_token->value,
        expression);
}
ast_node_t *parse_ast_node_expr_literal_numeric()
{

    token_t *num = advance_token();
    return make_ast_node_expr_literal_numeric((strtoimax(num->value, NULL, 10)));
}
void parse()
{
    printf("Starting parsing..\n");
    while (true)
    {
        token_t *tlc_node = peek_token();
        if (!tlc_node)
            break;
        switch (tlc_node->type)
        {
        case TOKEN_FUNCTION:
            add_ast_node(parse_ast_node_tlc_function_declaration());
            break;
        case TOKEN_TYPE:
            add_ast_node(parse_ast_node_tlc_declaration());
            break;
        default:
            panic("Unkown tlc token", tlc_node->value, tlc_node->linenumber);
            break;
        }
    }
    printf("Parsing completed!\n");
    debug(head);
}

// debug
void print_ast_node_tlc_declaration(ast_node_t *n)
{
    printf("AST_NODE_TLC_DECLARATION\n");
    print_ast_node_type(n->ast_node_tlc_declaration.type->kind);
    printf("%s \n", n->ast_node_tlc_declaration.name);
    printf("Expr: \n");
    print_ast_tree(n->ast_node_tlc_declaration.expr);
}

void print_ast_node_tlc_function_declaration(ast_node_t *n)
{
    printf("AST_NODE_TLC_FUNCTION_DECLARATION\n");
    print_ast_node_type(n->ast_node_tlc_function_declaration.return_type->kind);
    printf("%s \n", n->ast_node_tlc_function_declaration.name);
    printf("Params: \n");
    for (int i = 0; i < n->ast_node_tlc_function_declaration.param_count; i++)
    {
        print_ast_tree(n->ast_node_tlc_function_declaration.params[i]);
    }
    printf("\n");
    printf("Stmt_block: \n");
    print_ast_tree(n->ast_node_tlc_function_declaration.statements);
}

void print_ast_node_stmt_expression(ast_node_t *n)
{
    printf("AST_NODE_STMT_EXPR\n");
    print_ast_tree(n->ast_node_stmt_expression.expression);
}

void print_ast_node_expr_binary(ast_node_t *n)
{
    print_ast_tree(n->ast_node_expr_binary.left);
    switch (n->ast_node_expr_binary.operation)
    {
    case AST_NODE_OPERATION_ADDITION:
        printf("+\n");
        break;
    case AST_NODE_OPERATION_MULTIPLICATION:
        printf("*\n");
        break;
    case AST_NODE_OPERATION_SUBTRACTION:
        printf("-\n");
        break;
    case AST_NODE_OPERATION_DIVISION:
        printf("/\n");
        break;
    default:
        break;
    }
    print_ast_tree(n->ast_node_expr_binary.right);
}

void print_ast_node_expr_unary(ast_node_t *n)
{
    printf("AST_NODE_EXPR_UNARY\n");
    printf("Operation :%s \n", n->ast_node_expr_unary.operation);
    printf("value :%s \n", n->ast_node_expr_unary.value);
}

void debug(ast_node_t *root)
{
    while (root != NULL)
    {
        print_ast_tree(root);
        root = root->next;
    }
}

void print_ast_node_type(ast_node_type_kind_t t)
{
    switch (t)
    {
    case TYPE_INT:
        printf("INT\n");
        break;
    case TYPE_VOID:
        printf("VOID\n");
        break;
    default:
        break;
    }
}

void print_ast_node_stmt_block(ast_node_t *n)
{
    ast_node_t *stmt = n->ast_node_stmt_block.statements.first;
    for (int i = 0; i < n->ast_node_stmt_block.statements.count; i++)
    {
        print_ast_tree(stmt);
        stmt = stmt->next;
    }
}
void print_ast_node_expr_literal_numeric(ast_node_t *n)
{
    printf("%jd \n", n->ast_node_expr_literal_numeric.numeric_value);
}
void print_ast_node_expr_variable(ast_node_t *n)
{
    printf("%s \n", n->ast_node_expr_variable.name);
}

void print_ast_node_parameter(ast_node_t *n)
{
    print_ast_node_type(n->ast_node_parameter.type->kind);
    printf("%s ", n->ast_node_parameter.name);
}
void print_ast_node_assignment(ast_node_t *n)
{
    printf("Assignment\n");
    print_ast_node_type(n->ast_node_assignment.type->kind);
    printf("%s\n", n->ast_node_assignment.name);
    print_ast_tree(n->ast_node_assignment.expr);
}
void print_ast_node_reassignment(ast_node_t *n)
{
    printf("Reassignment\n");
    printf("%s\n", n->ast_node_reassignment.name);
    print_ast_tree(n->ast_node_reassignment.expr);
}
void print_ast_node_function_call(ast_node_t *n)
{
    printf("Function_call\n");
    printf("%s \n", n->ast_node_function_call.name);
    printf("Params: \n");
    for (int i = 0; i < n->ast_node_function_call.param_count; i++)
    {
        print_ast_tree(n->ast_node_function_call.params[i]);
    }
}
void print_ast_node_stmt_return(ast_node_t *n)
{
    printf("Return\n");
    print_ast_tree(n->ast_node_return.value);
}
void print_ast_tree(ast_node_t *n)
{
    if (!n)
    {
        return;
    }
    switch (n->type)
    {
    case AST_NODE_TLC_DECLARATION:
        print_ast_node_tlc_declaration(n);
        break;
    case AST_NODE_TLC_FUNCTION_DECLARATION:
        print_ast_node_tlc_function_declaration(n);
        break;
    case AST_NODE_EXPR_BINARY:
        print_ast_node_expr_binary(n);
        break;
    case AST_NODE_TYPE_STMT_EXPRESSION:
        print_ast_node_stmt_expression(n);
        break;
    case AST_NODE_EXPR_UNARY:
        print_ast_node_expr_unary(n);
        break;
    case AST_NODE_TYPE_STMT_BLOCK:
        print_ast_node_stmt_block(n);
        break;
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        print_ast_node_expr_literal_numeric(n);
        break;
    case AST_NODE_EXPR_LITERAL_VARIABLE:
        print_ast_node_expr_variable(n);
        break;
    case AST_NODE_PARAMETER:
        print_ast_node_parameter(n);
        break;
    case AST_NODE_TYPE_ASSIGNMENT:
        print_ast_node_assignment(n);
        break;
    case AST_NODE_TYPE_REASSIGNMENT:
        print_ast_node_reassignment(n);
        break;
    case AST_NODE_FUNCTION_CALL:
        print_ast_node_function_call(n);
        break;
    case AST_NODE_TYPE_STMT_RETURN:
        print_ast_node_stmt_return(n);
        break;
    default:
        printf("Uknown node type %d at print_ast_tree \n", n->type);
        exit(1);
    }
}