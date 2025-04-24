#pragma once
#include "parser.h"
#include "symbol.h"
#include <llvm-c/Core.h>
#include <stdio.h>
#include <stdlib.h>

LLVMTypeRef ir_make_type(ast_node_type_kind_t t);
LLVMValueRef ir_make_function_declaration(ast_node_t *n);
LLVMValueRef ir_make_expression(ast_node_t *n, symbol_table_t *symbols);
LLVMTypeRef *ir_make_parameters(ast_node_t **n, size_t count, symbol_table_t *symbols);
LLVMValueRef ir_evaluate_expression(ast_node_t *start, symbol_table_t *symbols);
LLVMValueRef ir_make_return(ast_node_t *n, symbol_table_t *symbols);
LLVMValueRef ir_store_value();

intmax_t evaluate(ast_node_t *node, symbol_table_t *symbols);
void ir_init();
