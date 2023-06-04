#include <mir/vreg.h>
#include <mir_gen.h>
#include <symbol_gen.h>

p_mir_vreg mir_vreg_gen(p_symbol_type p_type) {
    p_mir_vreg p_vreg = malloc(sizeof(*p_vreg));
    *p_vreg = (mir_vreg) {
        .p_type = p_type,
        .id = 0,
        .node = list_head_init(&p_vreg->node),
    };
    return p_vreg;
}

void mir_vreg_set_bb_def(p_mir_vreg p_vreg, p_mir_basic_block p_bb) {
    p_vreg->is_bb_param = true;
    p_vreg->p_bb_def = p_bb;
}
void mir_vreg_set_instr_def(p_mir_vreg p_vreg, p_mir_instr p_instr) {
    p_vreg->is_bb_param = false;
    p_vreg->p_instr_def = p_instr;
}

void mir_vreg_drop(p_mir_vreg p_vreg) {
    symbol_type_drop(p_vreg->p_type);
    free(p_vreg);
}
