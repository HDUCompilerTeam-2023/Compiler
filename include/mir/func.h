#ifndef __MIRFUNC__
#define __MIRFUNC__
#include <mir.h>

struct mir_func{
    p_mir_basic_block p_basic_block;
    p_symbol_sym p_func_sym;

    list_head temp_sym_head;
    p_mir_operand_list p_operand_list;
    list_head node;
};

#endif