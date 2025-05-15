#pragma once
#include <llvm-c/Core.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef struct
{
    const char *name;
    LLVMValueRef value;
    LLVMTypeRef type;
} symbol_entry_t;

typedef struct
{
    symbol_entry_t *entries;
    size_t count;
    size_t capacity;
} symbol_table_t;

symbol_table_t *symbol_table_create();
void symbol_table_free();
void symbol_table_add(symbol_table_t *t, symbol_entry_t e);
symbol_entry_t *symbol_table_find(symbol_table_t *t, symbol_table_t *g, const char *s);
void symbol_print_table(symbol_table_t *t, symbol_table_t *g);