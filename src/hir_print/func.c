#include <hir_print.h>
#include <stdio.h>

#include <hir/func.h>
#include <symbol/type.h>

void hir_func_print(p_hir_func p_func) {
    assert(p_func);

    hir_block_print(p_func->p_block);
}
