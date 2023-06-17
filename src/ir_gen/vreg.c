#include <ir/vreg.h>
#include <ir_gen.h>
#include <symbol_gen.h>

p_ir_vreg ir_vreg_gen(p_symbol_type p_type) {
    p_ir_vreg p_vreg = malloc(sizeof(*p_vreg));
    *p_vreg = (ir_vreg) {
        .p_type = p_type,
        .id = 0,
        .node = list_head_init(&p_vreg->node),
        .reg_id = -1,
    };
    return p_vreg;
}

void ir_vreg_set_bb_def(p_ir_vreg p_vreg, p_ir_basic_block p_bb) {
    p_vreg->is_bb_param = true;
    p_vreg->p_bb_def = p_bb;
}
void ir_vreg_set_instr_def(p_ir_vreg p_vreg, p_ir_instr p_instr) {
    p_vreg->is_bb_param = false;
    p_vreg->p_instr_def = p_instr;
}

p_ir_vreg ir_vreg_copy(p_ir_vreg p_vreg) {
    p_ir_vreg p_copy = malloc(sizeof(*p_copy));
    *p_copy = *p_vreg;
    p_copy->node = list_head_init(&p_copy->node);
    p_copy->p_type = symbol_type_copy(p_vreg->p_type);
    return p_copy;
}

void ir_vreg_drop(p_ir_vreg p_vreg) {
    symbol_type_drop(p_vreg->p_type);
    free(p_vreg);
}
