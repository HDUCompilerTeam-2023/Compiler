#include <hir_gen/func.h>

#include <hir_gen.h>

p_hir_func hir_func_gen(p_symbol_sym p_sym, p_hir_block p_block) {
    p_hir_func p_func = malloc(sizeof(*p_func));
    *p_func = (hir_func) {
        .p_sym = p_sym,
        .p_block = p_block,
        .node = list_head_init(&p_func->node),
    };
    return p_func;
}

void hir_func_drop(p_hir_func p_func) {
    assert(p_func);

    hir_block_drop(p_func->p_block);
    free(p_func);
}
