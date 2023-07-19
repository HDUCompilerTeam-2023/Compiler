#include <ir_gen.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <program/def.h>
#include <symbol_gen.h>

static inline bool if_need_cut(p_ir_basic_block_branch_target p_target) {
    p_list_head p_node1 = p_target->block_param.p_next;
    p_list_head p_node2 = p_target->p_block->basic_block_phis.p_next;
    while (p_node1 != &p_target->block_param) {
        p_ir_operand p_param = list_entry(p_node1, ir_bb_param, node)->p_bb_param;
        if (p_param->kind == imme)
            return true;
        p_ir_vreg p_phi = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        if (p_param->p_vreg->reg_id != p_phi->reg_id)
            return true;
        p_node1 = p_node1->p_next;
        p_node2 = p_node2->p_next;
    }
    return false;
}
static inline p_ir_basic_block cut_edge(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target, p_symbol_func p_func) {
    p_ir_basic_block p_new_basic_block = ir_basic_block_gen();
    ir_basic_block_insert_next(p_new_basic_block, p_basic_block);
    p_func->block_cnt++;
    p_new_basic_block->p_branch->kind = ir_br_branch;
    ir_basic_block_set_target1(p_basic_block, p_target);
    return p_new_basic_block;
}

void critical_edge_cut_pass(p_program p_ir) {
    p_list_head p_func_node;
    list_for_each(p_func_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_func_node, symbol_func, node);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
            if (p_basic_block->p_branch->kind == ir_cond_branch) {
                if (if_need_cut(p_basic_block->p_branch->p_target_1)) {
                    // 新建块处理target2 的 block_param
                    p_ir_basic_block p_new_basic_block = cut_edge(p_basic_block, p_basic_block->p_branch->p_target_1, p_func);
                    p_basic_block->p_branch->p_target_1 = ir_basic_block_branch_target_gen(p_new_basic_block);
                    ir_basic_block_add_prev_target(p_basic_block->p_branch->p_target_1, p_new_basic_block);
                }
                if (if_need_cut(p_basic_block->p_branch->p_target_2)) {
                    // 新建块处理target2 的 block_param
                    p_ir_basic_block p_new_basic_block = cut_edge(p_basic_block, p_basic_block->p_branch->p_target_2, p_func);
                    p_basic_block->p_branch->p_target_2 = ir_basic_block_branch_target_gen(p_new_basic_block);
                    ir_basic_block_add_prev_target(p_basic_block->p_branch->p_target_2, p_new_basic_block);
                }
            }
        }
        symbol_func_set_block_id(p_func);
    }
}