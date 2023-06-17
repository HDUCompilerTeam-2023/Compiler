#ifndef __IR_OPT_REG_ALLOC_WHOLE_IN_MEM__
#define __IR_OPT_REG_ALLOC_WHOLE_IN_MEM__
#include <ir.h>
typedef struct inmem_alloca_info inmem_alloca_info, *p_inmem_alloca_info;

struct inmem_alloca_info {
    size_t current_reg;
    size_t reg_num;
    p_symbol_var *pp_vmem; // 溢出变量
    p_symbol_func p_func;
};

void whole_in_mem_alloca(p_symbol_func p_func, size_t reg_num);

#endif
