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
symbol_entry_t *symbol_table_find(symbol_table_t *t, const char *s)
{
    for (size_t i = 0; i < t->count; i++)
    {
        if (strcmp(t->entries[i].name, s) == 0)
        {
            return &t->entries[i];
        }
    }
    return NULL;
}

void symbol_print_table(symbol_table_t *t)
{
    for (size_t i = 0; i < t->count; i++)
    {
        const char *name = t->entries[i].name;
        LLVMValueRef value = t->entries[i].value;

        char *value_str = LLVMPrintValueToString(value);
        printf("%zu : %s => %s\n", i, name, value_str);

        LLVMDisposeMessage(value_str);
    }
}