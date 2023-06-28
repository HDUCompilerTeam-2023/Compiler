#include <ir_opt/lir_gen/delete_cmp.h>

#include <program/def.h>
#include <symbol_gen.h>
#include <ir_gen.h>
static inline bool reg_if_param(p_ir_vreg p_vreg, p_symbol_func p_func) {
    return p_vreg->id < p_func->param_reg_cnt;
}

static inline bool if_int0(p_ir_operand p_operand) {
    if (p_operand->kind == imme)
        if (p_operand->p_type->ref_level == 0 && p_operand->p_type->basic == type_i32)
            if (p_operand->i32const == 0)
                return true;
    return false;
}

static inline bool can_update_tag(p_ir_vreg p_vreg, p_symbol_func p_func) {
    if (p_vreg->is_bb_param || reg_if_param(p_vreg, p_func))
        return false;
    p_ir_instr p_instr = p_vreg->p_instr_def;
    switch (p_instr->irkind) {
    case ir_unary:
        switch (p_instr->ir_unary.op) {
        case ir_val_assign:
            return true;
        default:
            return false;
        }
    case ir_binary:
        switch (p_instr->ir_binary.op) {
        case ir_mul_op:
        case ir_add_op:
        case ir_sub_op:
            return true;
        default:
            return false;
        }
    default:
        return false;
    }
}

void delete_cmp_pass(p_program p_ir){
    p_list_head p_func_node;
    list_for_each(p_func_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_func_node, symbol_func, node);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
            if (p_basic_block->p_branch->kind == ir_cond_branch) {
                assert(p_basic_block->p_branch->p_exp->kind == reg);
                p_ir_vreg p_cond = p_basic_block->p_branch->p_exp->p_vreg;
                assert(!p_cond->is_bb_param);
                assert(!reg_if_param(p_cond, p_func));

                p_ir_instr p_def_instr = p_cond->p_instr_def;
                assert(p_def_instr->irkind == ir_binary);
                assert(p_def_instr->ir_binary.p_src1->kind == reg);
                if (p_def_instr->ir_binary.op != ir_neq_op) continue;

                if (if_int0(p_def_instr->ir_binary.p_src2)) {
                    p_ir_vreg p_cmp_src = p_def_instr->ir_binary.p_src1->p_vreg;
                    if (can_update_tag(p_cmp_src, p_func)) {
                        p_cmp_src->if_cond = true;
                        p_basic_block->p_branch->p_exp->p_vreg = p_cmp_src;
                        ir_instr_drop(p_def_instr);
                        symbol_func_vreg_del(p_func, p_cond);
                    }
                }
            }
        }
        symbol_func_set_block_id(p_func);
        symbol_func_set_vreg_id(p_func);
    }
}
