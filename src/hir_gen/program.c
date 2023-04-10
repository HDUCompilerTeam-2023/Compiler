#include <hir_gen/program.h>

#include <hir_gen.h>

p_hir_program hir_program_gen(void) {
    p_hir_program p_program = malloc(sizeof(*p_program));
    *p_program = (hir_program) {
        .func = list_head_init(&p_program->func),
        .pss = symbol_store_initial(),
    };
    return p_program;
}
p_hir_program hir_program_add(p_hir_program p_program, p_hir_func p_func){
    list_add_prev(&p_func->node, &p_program->func);
    return p_program;
}
void hir_program_drop(p_hir_program p_program) {
    assert(p_program);
    while(!list_head_alone(&p_program->func)) {
        p_hir_func p_func = list_entry(p_program->func.p_next, hir_func, node);
        list_del(&p_func->node);
        hir_func_drop(p_func);
    }
    symbol_store_destroy(p_program->pss);
    free(p_program);
}
