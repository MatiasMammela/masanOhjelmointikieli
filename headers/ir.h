#pragma once
#include "parser.h"
#include "symbol.h"
#include <llvm-c/Core.h>
#include <stdio.h>
#include <stdlib.h>

LLVMTypeRef ir_make_type(ast_node_type_kind_t t);
LLVMValueRef ir_make_expression(ast_node_t *n, symbol_table_t *symbols);
void ir_make_tlc_function_declaration(ast_node_t *n);
void ir_make_tlc_declaration(ast_node_t *n);
void ir_store_parameter_values(ast_node_t **n, size_t count, LLVMValueRef func, symbol_table_t *symbols);
LLVMTypeRef *ir_get_parameter_types(ast_node_t **n, size_t count);
LLVMValueRef ir_make_function_call(ast_node_t *n, symbol_table_t *symbols);
void ir_init();