#include <ir/vreg.h>
#include <ir/bb_param.h>
#include <ir/basic_block.h>
#include <ir_gen/operand.h>
#include <symbol/func.h>

static inline void _do_phi(p_ir_bb_phi p_bb_phi) {
    p_ir_vreg p_vreg = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_bb_phi->p_basic_block->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target;
        assert(p_prev);
        assert(p_prev->p_block);
        assert(p_prev->p_block == p_bb_phi->p_basic_block);
        assert(p_prev->p_source_block);
        p_list_head p_node, p_node_src = p_prev->block_param.p_next;
        list_for_each(p_node, &p_bb_phi->p_basic_block->basic_block_phis) {
            assert(p_node_src != &p_prev->block_param);
            p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
            p_ir_bb_param p_param = list_entry(p_node_src, ir_bb_param, node);
            assert(p_param->p_bb_param->kind == reg);
            if (p_phi == p_bb_phi) {
                assert(p_param->p_bb_param->kind == reg);
                if (p_param->p_bb_param->p_vreg == p_bb_phi->p_bb_phi)
                    break;
                if (p_vreg && p_vreg != p_param->p_bb_param->p_vreg)
                    return;
                if (!p_vreg)
                    p_vreg = p_param->p_bb_param->p_vreg;
                break;
            }
            p_node_src = p_node_src->p_next;
        }
        assert(p_node_src != &p_prev->block_param);
    }
    assert(p_vreg); // need 1 input (no self assign) at least

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_bb_phi->p_bb_phi->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_bb_phi->p_bb_phi);
        ir_operand_reset_vreg(p_use, p_vreg);
    }
}

void ir_func_phi_propagation(p_symbol_func p_func) {
    assert(p_func->p_entry_block);
    p_list_head p_node;
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->def_type != bb_phi_def)
            continue;

        _do_phi(p_vreg->p_bb_phi);
    }
}

void ir_phi_propagation(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        ir_func_phi_propagation(p_func);
    }
}
