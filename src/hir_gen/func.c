#include <hir_gen/func.h>

#include <hir_gen.h>

#include <symbol/sym.h>

p_hir_func hir_func_gen(p_symbol_item p_item, p_hir_block p_block) {
    p_hir_func p_func = malloc(sizeof(*p_func));
    *p_func = (hir_func) {
        .p_block = p_block,
        .p_sym = p_item->p_info,
        .node = list_init_head(&p_func->node),
    };
    p_item->p_func = p_func;
    return p_func;
}

void hir_func_drop(p_hir_func p_func) {
    assert(p_func);
    list_del(&p_func->node);

    if(p_func->p_block)
        hir_block_drop(p_func->p_block);
    free(p_func);
}
