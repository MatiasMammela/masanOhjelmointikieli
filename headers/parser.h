#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
typedef enum
{
    // Top level
    AST_NODE_TLC_DECLARATION,
    AST_NODE_TLC_FUNCTION_DELCARATION,
    // Statements
    AST_NODE_TYPE_STMT_DECLARATION,
    AST_NODE_TYPE_STMT_EXPRESSION,
    AST_NODE_TYPE_STMT_BLOCK,
    // Expressions
    AST_NODE_EXPR_LITERAL_NUMERIC,
    AST_NODE_EXPR_BINARY,
    AST_NODE_EXPR_LITERAL_VARIABLE,
} ast_node_type_t;

typedef enum
{
    AST_NODE_OPERATION_ASSIGN,
    AST_NODE_OPERATION_ADDITION,
    AST_NODE_OPERATION_SUBTRACTION,
    AST_NODE_OPERATION_MULTIPLICATION,
    AST_NODE_OPERATION_DIVISION,
} ast_node_operation_t;
typedef struct ast_node ast_node_t;
typedef struct ast_type ast_type_t;
typedef struct ast_parameter ast_parameter_t;
typedef struct
{
    size_t count;
    ast_node_t *first, *last;
} ast_node_list_t;

struct ast_type
{
    struct
    {
        bool is_signed;
        size_t bit_size;
    } integer;
};
struct ast_parameter
{
    ast_type_t *type;
    const char *name;
};
struct ast_node
{
    ast_node_type_t type;
    ast_node_t *next;
    union
    {
        struct
        {
            const char *name;
        } ast_node_expr_variable;
        struct
        {
            ast_type_t *type;
            const char *name;
            ast_node_t *expr;
        } ast_node_tlc_declaration;
        struct
        {
            ast_node_t *expression;
        } ast_node_stmt_expression;
        struct
        {
            uintmax_t numeric_value;
        } ast_node_expr_literal_numeric;

        struct
        {
            ast_node_list_t statements;
        } ast_node_stmt_block;

        struct
        {
            ast_node_t *left;
            ast_node_operation_t operation;
            ast_node_t *right;
        } ast_node_expr_binary;
        struct
        {
            ast_type_t *return_type;
            const char *name;
            ast_parameter_t **params;
            ast_node_t *statements; // Point to stmt block
        } ast_node_tlc_function_declaration;
    };
};

// make
ast_node_t *make_ast_node_expr_literal_numeric(uintmax_t value);
ast_node_t *make_ast_node_expr_binary(ast_node_t *left, ast_node_operation_t operation, ast_node_t *right);
ast_node_t *make_ast_node_stmt_expression(ast_node_t *expr);
ast_node_t *make_ast_node_tlc_declaration(ast_type_t *type, const char *name, ast_node_t *expr);
ast_node_t *make_ast_node_expr_literal_variable(const char *name);
ast_node_t *make_ast_node_tlc_function_declaration(ast_type_t *return_type, const char *name, ast_parameter_t **params, ast_node_t *statements);
ast_node_t *make_ast_node_stmt_block(ast_node_list_t stmt_list);
// parse
ast_type_t *parse_ast_type(const char *value);
ast_node_t *parse_ast_node_stmt_expression();
ast_node_t *parse_ast_node_tlc_declaration();
ast_node_t *parse_ast_node_expr_binary();
ast_node_t *parse_ast_node_expr_literal_variable();
ast_node_t *parse_ast_node_expr_literal_numeric();
ast_node_t *parse_ast_node_tlc_function_declaration();
ast_node_t *parse_ast_node_stmt_block();
// list
ast_node_list_t *ast_node_list_create();
void ast_node_list_append(ast_node_list_t *list, ast_node_t *node);

// main
void parse();