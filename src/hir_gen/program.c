#include <hir_gen/program.h>

#include <hir_gen.h>

p_hir_program hir_program_gen(void) {
    p_hir_program p_program = malloc(sizeof(*p_program));
    *p_program = (hir_program) {
        .pss = symbol_store_initial(),
    };
    return p_program;
}
void hir_program_drop(p_hir_program p_program) {
    assert(p_program);
    symbol_store_destroy(p_program->pss);
    free(p_program);
}
