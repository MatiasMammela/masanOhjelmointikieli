#pragma once
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
typedef struct token_t token_t;

typedef enum
{
    TOKEN_FUNCTION, // 0 works
    TOKEN_RETURN,
    TOKEN_NUMBER,     // 1 works
    TOKEN_IDENTIFIER, // 3 works
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_EQUALS,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_COMMA,
    TOKEN_PERIOD,
    TOKEN_SEMICOLON,
    TOKEN_TYPE,
} token_type_t;

struct token_t
{
    token_type_t type;
    const char *value;
    token_t *next;
    uint32_t linenumber;
};

typedef struct
{
    const char *regex;
    token_type_t type;
} pattern_t;

static pattern_t patterns[] = {
    {"^fn\\b", TOKEN_FUNCTION},
    {"^return\\b", TOKEN_RETURN},
    {"^\\d+", TOKEN_NUMBER},
    {"^[a-zA-Z_][a-zA-Z0-9_]*", TOKEN_IDENTIFIER},
    {"^\\(", TOKEN_LPAREN},
    {"^\\)", TOKEN_RPAREN},
    {"^\\{", TOKEN_LBRACKET},
    {"^\\}", TOKEN_RBRACKET},
    {"^\\=", TOKEN_EQUALS},
    {"^\\+", TOKEN_PLUS},
    {"^\\-", TOKEN_MINUS},
    {"^\\/", TOKEN_SLASH},
    {"^\\*", TOKEN_STAR},
    {"^,", TOKEN_COMMA},
    {"^\\.", TOKEN_PERIOD},
    {"^;", TOKEN_SEMICOLON},
};

void lexer_parse();
void print_tokens();
void match_types();
token_type_t is_type();

// Move / check current /return current
token_t *advance_token();
token_t *advance_assert(const char c);
token_t *advance_type();
token_t *advance_assert_token_type(token_type_t t);
token_t *advance_operation();

// Dont move / check current / return current
token_t *peek_token();
token_t *peek_type();
token_t *peek_next();
token_t *peek_operation();
