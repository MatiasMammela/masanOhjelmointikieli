#include "lexer.h"
#include "types.h"
#include <stdio.h>
const char *input;
size_t pos = 0;
token_t *tokens;
uint32_t linenumber = 1;
static token_t *head = NULL;
static token_t *tail = NULL;

void add_token(token_type_t type, const char *value)
{

    token_t *new_token = (token_t *)malloc(sizeof(token_t));
    if (!new_token)
    {
        printf("Failed to allocate memory for a token at add_token\n");
        exit(1);
    }

    new_token->type = type;
    new_token->value = strdup(value);
    new_token->next = NULL;
    new_token->linenumber = linenumber;
    // Possible type
    if (type == TOKEN_IDENTIFIER)
    {
        new_token->type = is_type(new_token);
    }

    if (tail)
    {
        tail->next = new_token;
    }
    else
    {
        head = new_token;
    }
    tail = new_token;
}

int match_and_add_token(const char *pattern, token_type_t type)
{
    int err_num;
    PCRE2_SIZE err_offset;

    pcre2_code *re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0, &err_num, &err_offset, NULL);
    if (re == NULL)
    {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(err_num, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", (int)err_offset, buffer);
        return 0;
    }

    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
    int rc = pcre2_match(
        re,
        (PCRE2_SPTR)(input + pos),
        strlen(input + pos),
        0, // start at offset 0 within input + pos
        0, // default options
        match_data,
        NULL);

    if (rc < 0)
    {
        pcre2_code_free(re);
        pcre2_match_data_free(match_data);
        return 0;
    }

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    PCRE2_SIZE start = ovector[0];
    PCRE2_SIZE end = ovector[1];
    size_t match_len = end - start;

    char *token_str = malloc(match_len + 1);
    strncpy(token_str, input + pos + start, match_len);
    token_str[match_len] = '\0';
    add_token(type, token_str);
    pos += match_len;

    pcre2_code_free(re);
    pcre2_match_data_free(match_data);
    return 1;
}

void skip_whitespace()
{

    while (input[pos] == ' ' || input[pos] == '\n')
    {
        if (input[pos] == '\n')
        {
            linenumber++;
        }
        pos++;
    }
}

void skip_chars()
{
    for (size_t i = 0; i < sizeof(patterns) / sizeof(pattern_t); i++)
    {
        if (match_and_add_token(patterns[i].regex, patterns[i].type))
        {
            return;
        }
    }
    pos++;
}
void lexer_parse(const char *content)
{

    input = content;
    pos = 0;
    while (pos < strlen(input))
    {
        skip_whitespace();
        skip_chars();
    }
}

void print_tokens()
{
    token_t *current = head;
    while (current)
    {
        printf("Token: Lnum=%d, Type=%d, Value='%s'\n", current->linenumber, current->type, current->value);
        current = current->next;
    }
}

token_type_t is_type(token_t *token)
{
    for (int i = 0; i < NUM_TYPES; i++)
    {
        if (strcmp(token->value, types[i]) == 0)
        {
            return TOKEN_TYPE;
        }
    }
    return TOKEN_IDENTIFIER;
}

int is_operation(token_t *token)
{
    switch (token->type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_STAR:
    case TOKEN_SLASH:
    case TOKEN_EQUALS:
        return 1;
        break;

    default:
        return 0;
        break;
    }
}

// Peek

token_t *peek_type()
{
    if (head != NULL && head->type == TOKEN_TYPE)
    {
        return head;
    }
    fprintf(stderr, "Expected TOKEN_TYPE got %d \n", head->type);
    return NULL;
}

token_t *peek_token()
{
    if (head != NULL)
    {
        return head;
    }
    return NULL;
}
token_t *peek_next()
{
    if (head->next != NULL)
    {
        return head->next;
    }
    return NULL;
}

token_t *peek_operation()
{
    if (head != NULL && is_operation(head))
    {
        return head;
    }
    return NULL;
}

// Advance

token_t *advance_token()
{
    token_t *t;
    if (head != NULL)
    {
        t = head;
        head = head->next;
        return t;
    }
    return NULL;
}
token_t *advance_assert(const char c)
{
    token_t *t;
    if (head != NULL && head->value[0] == c)
    {
        t = head;
        head = head->next;
        return t;
    }
    fprintf(stderr, "Expected %c got %c \n", c, head->value[0]);
    exit(1);
}

token_t *advance_operation()
{
    token_t *t;
    if (head != NULL && is_operation(head))
    {
        t = head;
        head = head->next;
        return t;
    }
    fprintf(stderr, "Expected OPERATION got %d \n", head->type);
    exit(1);
}
token_t *advance_assert_token_type(token_type_t type)
{
    token_t *t;
    if (head != NULL && head->type == type)
    {
        t = head;
        head = head->next;
        return t;
    }
    fprintf(stderr, "Expected %d got %d \n", type, head->type);
    exit(1);
}

token_t *advance_type()
{
    token_t *t;
    if (head != NULL && head->type == TOKEN_TYPE)
    {
        t = head;
        head = head->next;
        return t;
    }
    fprintf(stderr, "Expected TOKEN_TYPE got %d \n", head->type);
    exit(1);
}
