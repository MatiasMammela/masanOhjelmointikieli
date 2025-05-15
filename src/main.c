#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "ir.h"
int main(void)
{
    FILE *file_ptr;
    file_ptr = fopen("resources/code.txt", "r");
    if (!file_ptr)
    {
        printf("Error opening a file in main");
        return 1;
    }
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    rewind(file_ptr);

    char *buffer = malloc(file_size + 1);
    if (!buffer)
    {
        printf("Error allocating memory for a file in main");
        fclose(file_ptr);
        return 1;
    }

    fread(buffer, 1, file_size, file_ptr);
    buffer[file_size] = '\0';

    fclose(file_ptr);

    lexer_parse(buffer);
    free(buffer);
    print_tokens();

    parse();
    ir_init();
    return 0;
}