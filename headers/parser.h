#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "lexer.h"
typedef enum
{
    // Top level
    AST_NODE_TLC_DECLARATION,
    AST_NODE_TLC_FUNCTION_DECLARATION,
    // Statements
    AST_NODE_TYPE_STMT_DECLARATION,
    AST_NODE_TYPE_STMT_EXPRESSION,
    AST_NODE_TYPE_STMT_BLOCK,
    AST_NODE_TYPE_REASSIGNMENT,
    AST_NODE_TYPE_ASSIGNMENT,
    AST_NODE_TYPE_STMT_RETURN,
    // Expressions
    AST_NODE_EXPR_LITERAL_NUMERIC,
    AST_NODE_EXPR_BINARY,
    AST_NODE_EXPR_LITERAL_VARIABLE,
    AST_NODE_EXPR_UNARY,
    AST_NODE_PARAMETER,

    AST_NODE_FUNCTION_CALL,
} ast_node_type_t;

typedef enum
{
    TYPE_INT,
    TYPE_VOID,
} ast_node_type_kind_t;

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
    ast_node_type_kind_t kind;
    struct
    {
        bool is_signed;
        size_t bit_size;
    } integer;
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
        } ast_node_parameter;
        struct
        {
            ast_type_t *type;
            const char *name;
            ast_node_t *expr;
        } ast_node_tlc_declaration;
        struct
        {
            ast_type_t *type;
            const char *name;
            ast_node_t *expr;
        } ast_node_assignment;
        struct
        {
            const char *name;
            ast_node_t *expr;
        } ast_node_reassignment;
        struct
        {
            ast_node_t *value;
        } ast_node_return;
        struct
        {
            ast_node_t *expression;
        } ast_node_stmt_expression;
        struct
        {
            intmax_t numeric_value;
        } ast_node_expr_literal_numeric;
        struct
        {
            const char *name;
            ast_node_t **params;
            size_t param_count;
        } ast_node_function_call;
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
            ast_node_t **params;
            size_t param_count;
            ast_node_t *statements; // Point to stmt block
        } ast_node_tlc_function_declaration;
        struct
        {
            ast_node_operation_t operation;
            ast_node_t *value;
        } ast_node_expr_unary;
    };
};

// make
ast_node_t *make_ast_node_expr_literal_numeric(intmax_t value);
ast_node_t *make_ast_node_expr_binary(ast_node_t *left, ast_node_operation_t operation, ast_node_t *right);
ast_node_t *make_ast_node_stmt_expression(ast_node_t *expr);
ast_node_t *make_ast_node_tlc_declaration(ast_type_t *type, const char *name, ast_node_t *expr);
ast_node_t *make_ast_node_expr_literal_variable(const char *name);
ast_node_t *make_ast_node_tlc_function_declaration(ast_type_t *return_type, const char *name, ast_node_t **params, size_t param_count, ast_node_t *statements);
ast_node_t *make_ast_node_stmt_block(ast_node_list_t stmt_list);
ast_node_t *make_ast_node_reassignment(const char *name, ast_node_t *expr);
ast_node_t *make_ast_node_expr_unary(ast_node_operation_t operation, ast_node_t *value);
ast_node_t *make_ast_node_assignment(ast_type_t *type, const char *name, ast_node_t *expr);
ast_node_t *make_ast_node_return(ast_node_t *value);
// parse
ast_type_t *parse_ast_type(token_t *t);
ast_node_t *parse_ast_node_stmt_expression();
ast_node_t *parse_ast_node_tlc_declaration();
ast_node_t *parse_ast_node_expr_binary();
ast_node_t *parse_ast_node_expr_literal_variable();
ast_node_t *parse_ast_node_expr_literal_numeric();
ast_node_t *parse_ast_node_tlc_function_declaration();
ast_node_t *parse_ast_node_stmt_block();
ast_node_t *parse_ast_node_reassignment();
ast_node_t *parse_ast_node_expr_unary();
ast_node_t *parse_ast_node_assignment();
ast_node_t *parse_ast_node_return();
ast_node_t *parse_ast_node_function_call_params();
ast_node_t *parse_ast_node_function_call();
ast_node_t *parse_ast_node_expr_primary();
// list
ast_node_list_t *ast_node_list_create();
void ast_node_list_append(ast_node_list_t *list, ast_node_t *node);

// debug
void print_ast_node_expr_literal_numeric(ast_node_t *n);
void print_ast_node_expr_binary(ast_node_t *n);
void print_ast_node_stmt_expression(ast_node_t *n);
void print_ast_node_tlc_declaration(ast_node_t *n);
void print_ast_node_tlc_function_declaration(ast_node_t *n);
void print_ast_node_stmt_expr(ast_node_t *n);
void print_ast_tree(ast_node_t *n);
void debug(ast_node_t *n);
void print_ast_node_type(ast_node_type_kind_t t);
// main
void parse();
extern ast_node_t *head;