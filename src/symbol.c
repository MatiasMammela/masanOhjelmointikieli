#include "symbol.h"

symbol_table_t *symbol_table_create()
{
    symbol_table_t *t = malloc(sizeof(symbol_table_t));
    t->capacity = 1;
    t->count = 0;
    t->entries = malloc(sizeof(symbol_entry_t) * t->capacity);
    return t;
}
void symbol_table_free(symbol_table_t *t)
{
    free(t->entries);
    free(t);
}
void symbol_table_add(symbol_table_t *t, symbol_entry_t e)
{
    if (t->count >= t->capacity)
    {
        t->capacity = t->capacity ? t->capacity * 2 : 4;
        t->entries = realloc(t->entries, t->capacity * sizeof(symbol_entry_t));
    }
    t->entries[t->count] = e;
    t->count++;
}
symbol_entry_t *symbol_table_find(symbol_table_t *t, symbol_table_t *g, const char *s)
{
    for (size_t i = 0; i < t->count; i++)
    {
        if (strcmp(t->entries[i].name, s) == 0)
        {
            return &t->entries[i];
        }
    }

    for (size_t i = 0; i < g->count; i++)
    {
        if (strcmp(g->entries[i].name, s) == 0)
        {
            return &g->entries[i];
        }
    }

    return NULL;
}

void symbol_print_table(symbol_table_t *t, symbol_table_t *g)
{

    size_t global_count = 0;

    for (size_t i = 0; i < t->count; i++)
    {
        const char *name = t->entries[i].name;
        LLVMValueRef value = t->entries[i].value;
        LLVMTypeRef type = t->entries[i].type;
        char *value_str = LLVMPrintValueToString(value);
        char *type_str = LLVMPrintTypeToString(type);
        printf("%zu : %s => %s %s\n", global_count, name, type_str, value_str);

        LLVMDisposeMessage(value_str);
        LLVMDisposeMessage(type_str);
        global_count++;
    }

    for (size_t i = 0; i < g->count; i++)
    {
        const char *name = g->entries[i].name;
        LLVMValueRef value = g->entries[i].value;
        LLVMTypeRef type = g->entries[i].type;
        char *value_str = LLVMPrintValueToString(value);
        char *type_str = LLVMPrintTypeToString(type);
        printf("%zu : %s => %s %s\n", global_count, name, type_str, value_str);

        LLVMDisposeMessage(value_str);
        LLVMDisposeMessage(type_str);
        global_count++;
    }
}