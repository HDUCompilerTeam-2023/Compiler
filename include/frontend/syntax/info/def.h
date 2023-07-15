#ifndef __FRONTEND_SYNTAX_INFO_DEF__
#define __FRONTEND_SYNTAX_INFO_DEF__

#include <frontend/syntax/info/use.h>

struct syntax_info {
    p_symbol_table p_table;
    p_program p_program;
    p_symbol_func p_func;
    p_ast_block p_block;
    p_symbol_func p_mem_set;
};

#endif
