#include <hir_print.h>
#include <stdio.h>

#include <hir/func.h>
#include <hir/symbol.h>
#include <hir/type.h>

void hir_func_print(p_hir_func p_func) {
    assert(p_func);

    printf("\n");
    hir_block_print(p_func->p_block);
}
