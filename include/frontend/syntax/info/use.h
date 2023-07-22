#ifndef __FRONTEND_SYNTAX_INFO_USE__
#define __FRONTEND_SYNTAX_INFO_USE__

#include <frontend/syntax/symbol_table/use.h>
#include <frontend/syntax/decl_head/use.h>
#include <frontend/syntax/decl/use.h>
#include <program/use.h>

typedef struct syntax_info syntax_info, *p_syntax_info;

p_program syntax_info_get_program(p_syntax_info p_info);

void syntax_set_block(p_syntax_info p_info, p_ast_block p_block);

void syntax_zone_push(p_syntax_info p_info);
void syntax_zone_pop(p_syntax_info p_info);
p_symbol_var syntax_find_var(p_syntax_info p_info, const char *name);
p_symbol_func syntax_find_func(p_syntax_info p_info, const char *name);
p_symbol_str syntax_get_str(p_syntax_info p_info, const char *string);

void syntax_set_func(p_syntax_info p_info, p_symbol_func p_func);
void syntax_func_add_variable(p_syntax_info p_info, p_symbol_var p_var);
void syntax_func_add_param(p_syntax_info p_info, p_symbol_var p_var);

void syntax_program_add_variable(p_syntax_info p_info, p_symbol_var p_var);
void syntax_program_add_function(p_syntax_info p_info, p_symbol_func p_func);

p_syntax_decl_head syntax_declaration(p_syntax_info p_info, p_syntax_decl_head p_head, p_syntax_decl p_decl);

void syntax_func_head(p_syntax_info p_info, basic_type type, char *name);
void syntax_func_end(p_syntax_info p_info, p_ast_block p_block);

void syntax_rtlib_func_init(p_syntax_info p_info);

#endif
