#include "parser.h"
#include "lexer.h"
ast_node_t *nodes;
ast_node_t *head = NULL;
ast_node_t *tail = NULL;
void add_ast_node(ast_node_t *n)
{
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

ast_node_t *make_ast_node_expr_literal_numeric(uintmax_t value)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_LITERAL_NUMERIC;
    n->ast_node_expr_literal_numeric.numeric_value = value;
    return n;
}

ast_node_t *parse_ast_node_expr_literal_numeric()
{
    token_t *num = advance_token();
    return make_ast_node_expr_literal_numeric((strtoumax(num->value, NULL, 10)));
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

ast_node_t *make_ast_node_tlc_declaration(ast_type_t *type, const char *name, ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TLC_DECLARATION;
    n->ast_node_tlc_declaration.type = type;
    n->ast_node_tlc_declaration.name = name;
    n->ast_node_tlc_declaration.expr = expr;
    return n;
}

ast_type_t *parse_ast_type(const char *value)
{
    ast_type_t *type = malloc(sizeof(ast_type_t));
    if (value == "int")
    {
        type->integer.is_signed = true;
        type->integer.bit_size = 32;
    }
    return type;
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

ast_node_t *parse_ast_node_tlc_function_declaration()
{
    token_t *token_fn = advance_assert_token_type(TOKEN_FUNCTION);
    printf("%s\n", token_fn->value);
    token_t *token_type = advance_type();
    printf("%s\n", token_type->value);
    token_t *token_name = advance_token();
    printf("%s\n", token_name->value);
    token_t *token = advance_token();
    printf("%s\n", token->value);

    ast_parameter_t **params = NULL;
    size_t param_count = 0;
    size_t param_capacity = 0;
    while (token->type != TOKEN_RPAREN)
    {
        if (param_count == param_capacity)
        {
            // if 0 set to 4 else double
            param_capacity = (param_capacity == 0) ? 4 : param_capacity * 2;
            ast_parameter_t **new_params = realloc(params, sizeof(ast_parameter_t *) * param_capacity);
            if (!new_params)
            {
                perror("Failed to realloc for function parameters!!\n");
                exit(1);
            }
            params = new_params;
        }

        ast_parameter_t *param = malloc(sizeof(ast_parameter_t));
        token_t *type = advance_type();
        printf("Type  %s\n", type->value);
        token_t *name = advance_token();
        printf("Name  %s\n", name->value);
        param->type = parse_ast_type(type->value);
        param->name = name->value;
        params[param_count] = param;
        token_t *comma = advance_token();
        if (comma->type != TOKEN_COMMA)
        {
            printf("Not Comma \n");
            break;
        }
        param_count++;
    }
    ast_node_t *statements = parse_ast_node_stmt_block();

    return make_ast_node_tlc_function_declaration(
        parse_ast_type(token_type->value),
        token_name->value,
        params,
        statements);
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

        expect_token();

        // Get the right

        token_t *right_token = peek_token();
        ast_node_t *right_node;
        switch (right_token->type)
        {
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

        // convert token to binary operation
        ast_node_operation_t operation_type;
        switch (operation->type)
        {
        case TOKEN_PLUS:
            operation_type = AST_NODE_OPERATION_ADDITION;
            break;
        case TOKEN_MINUS:
            operation_type = AST_NODE_OPERATION_SUBTRACTION;
            break;
        case TOKEN_STAR:
            operation_type = AST_NODE_OPERATION_MULTIPLICATION;
            break;
        case TOKEN_SLASH:
            operation_type = AST_NODE_OPERATION_DIVISION;
            break;
        default:
            break;
        }
        left = make_ast_node_expr_binary(left, operation_type, right_node);
    }
    return left;
}

ast_node_t *make_ast_node_expr_literal_variable(const char *name)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_EXPR_LITERAL_VARIABLE;
    n->ast_node_expr_variable.name = name;
    return n;
}

ast_node_t *parse_ast_node_expr_literal_variable()
{
    token_t *name = advance_token();
    return make_ast_node_expr_literal_variable(name->value);
}
void print_ast(ast_node_t *node, int indent)
{
    if (!node)
    {
        return;
    }

    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }

    switch (node->type)
    {
    case AST_NODE_EXPR_LITERAL_NUMERIC:
        printf("Literal: %ju\n", node->ast_node_expr_literal_numeric.numeric_value);
        break;

    case AST_NODE_EXPR_LITERAL_VARIABLE:
        printf("Variable: %s\n", node->ast_node_expr_variable.name);
        break;

    case AST_NODE_EXPR_BINARY:
        printf("Binary Operation: ");
        switch (node->ast_node_expr_binary.operation)
        {
        case AST_NODE_OPERATION_ADDITION:
            printf("+\n");
            break;
        case AST_NODE_OPERATION_SUBTRACTION:
            printf("-\n");
            break;
        case AST_NODE_OPERATION_MULTIPLICATION:
            printf("*\n");
            break;
        case AST_NODE_OPERATION_DIVISION:
            printf("/\n");
            break;
        default:
            printf("Unknown\n");
            break;
        }
        print_ast(node->ast_node_expr_binary.left, indent + 1);
        print_ast(node->ast_node_expr_binary.right, indent + 1);
        break;

    default:
        for (int i = 0; i < indent; i++)
            printf("  ");
        printf("Unknown node type %d\n", node->type);
        break;
    }
}

ast_node_t *parse_ast_node_stmt_expression()
{
    token_t *left_token = peek_token();
    ast_node_t *left_node;
    switch (left_token->type)
    {
    case TOKEN_IDENTIFIER:
        left_node = parse_ast_node_expr_literal_variable();
        break;
    case TOKEN_NUMBER:
        left_node = parse_ast_node_expr_literal_numeric();
        break;
    default:
        break;
    }

    token_t *left_next = left_token->next;
    if (left_next->type != TOKEN_SEMICOLON)
    {
        ast_node_t *expr = parse_ast_node_expr_binary(left_node, 0);
        printf("Parsed binary expression:\n");
        print_ast(expr, 0);
        token_t *cuurr = peek_token();
        token_t *sem = advance_assert_token_type(TOKEN_SEMICOLON);
        return expr;
    }
    else
    {
        token_t *sem = advance_assert_token_type(TOKEN_SEMICOLON);
        return left_node;
    }

    // if left is value and its next is not ; its part of an binary expr

    // if left is a value and its next is a  ; its a variable
}

ast_node_t *parse_ast_node_tlc_declaration()
{

    token_t *type_token = expect_token();
    token_t *name_token = advance_token();
    expect_token();
    ast_node_t *expression = parse_ast_node_stmt_expression();
    return make_ast_node_tlc_declaration(
        parse_ast_type(type_token->value),
        name_token->value,
        expression);
}

ast_node_t *make_ast_node_stmt_expression(ast_node_t *expr)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_STMT_EXPRESSION;
    n->ast_node_tlc_declaration.expr = expr;
    return n;
}
ast_node_t *make_ast_node_tlc_function_declaration(ast_type_t *return_type, const char *name, ast_parameter_t **params, ast_node_t *statements)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TLC_FUNCTION_DELCARATION;
    n->ast_node_tlc_function_declaration.return_type = return_type;
    n->ast_node_tlc_function_declaration.name = name;
    n->ast_node_tlc_function_declaration.params = params;
    n->ast_node_tlc_function_declaration.statements = statements;
    return n;
}

ast_node_t *parse_ast_node_stmt_block()
{
    token_t *lbracket = advance_assert_token_type(TOKEN_LBRACKET);
    token_t *token = peek_token();
    ast_node_list_t *statements = ast_node_list_create();
    while (token->type != TOKEN_RBRACKET)
    {
        printf("%s\n", token->value);
        // Parse function body
        switch (token->type)
        {
        case TOKEN_TYPE:
            ast_node_t *decl = parse_ast_node_tlc_declaration();
            ast_node_list_append(statements, decl);
            break;
        default:
            break;
        }
        token = peek_token();
    }
    advance_assert_token_type(TOKEN_RBRACKET);
    return make_ast_node_stmt_block(*statements);
}

ast_node_list_t *ast_node_list_create()
{
    ast_node_list_t *l = malloc(sizeof(ast_node_list_t));
    l->count = 0;
    l->first = NULL;
    l->last = NULL;
    return l;
}

ast_node_t *make_ast_node_stmt_block(ast_node_list_t stmt_list)
{
    ast_node_t *n = malloc(sizeof(ast_node_t));
    n->type = AST_NODE_TYPE_STMT_BLOCK;
    n->ast_node_stmt_block.statements = stmt_list;
    return n;
}

void ast_node_list_append(ast_node_list_t *list, ast_node_t *node)
{
    if (list->first == NULL)
    {
        list->first = node;
    }
    else if (tail)
    {
        list->last->next = node;
    }
    list->last = node;
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
            printf("Function found at tlc\n");
            add_ast_node(parse_ast_node_tlc_function_declaration());
            break;
        case TOKEN_TYPE:
            printf("declaration found at tlc\n");
            add_ast_node(parse_ast_node_tlc_declaration());
            break;
        default:
            return;
            break;
        }
    }
    printf("Parsing completed!\n");
}
