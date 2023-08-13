#include <ir/bb_param.h>
#include <ir_gen/basic_block.h>
#include <ir_gen/operand.h>
#include <ir_gen/varray.h>
#include <ir_gen/vreg.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <ir_manager/set_cond.h>
#include <program/def.h>
#include <program/use.h>
#include <symbol_gen/func.h>

static inline void _ir_opt_loop_pre_head_loop(p_symbol_func p_func, p_nestedtree_node p_loop) {
    p_list_head p_node;
    list_for_each(p_node, &p_loop->son_list) {
        p_nestedtree_node p_loop = list_entry(p_node, nested_list_node, node)->p_nested_node;
        assert(p_loop->depth);
        _ir_opt_loop_pre_head_loop(p_func, p_loop);
    }

    p_ir_basic_block p_loop_head = p_loop->head;
    assert(p_loop_head->p_nestree_node == p_loop);

    p_ir_branch_target_node *target_node_map = malloc(sizeof(void *) * p_func->block_cnt * 2);
    size_t target_node_cnt = 0;
    list_for_each(p_node, &p_loop_head->prev_branch_target_list) {
        p_ir_branch_target_node p_prev_node = list_entry(p_node, ir_branch_target_node, node);
        assert(p_prev_node->p_target->p_block == p_loop_head);
        if (!ir_basic_block_dom_check(p_prev_node->p_target->p_source_block, p_prev_node->p_target->p_block)) {
            target_node_map[target_node_cnt++] = p_prev_node;
        }
    }

    assert(target_node_cnt || p_loop_head == p_func->p_entry_block);
    if (target_node_cnt == 1)
        goto exit;

    p_ir_basic_block p_prev_head = ir_basic_block_gen();
    ir_basic_block_insert_prev(p_prev_head, p_loop_head);
    ir_basic_block_set_br(p_prev_head, p_loop_head);
    list_for_each(p_node, &p_loop_head->basic_block_phis) {
        p_ir_bb_phi p_des_phi = list_entry(p_node, ir_bb_phi, node);
        assert(p_des_phi->p_basic_block == p_loop_head);
        p_ir_vreg p_des_vreg = p_des_phi->p_bb_phi;
        assert(p_des_vreg->def_type == bb_phi_def);
        assert(p_des_vreg->p_bb_phi == p_des_phi);
        p_ir_vreg p_new_vreg = ir_vreg_copy(p_des_vreg);
        symbol_func_vreg_add(p_func, p_new_vreg);
        ir_basic_block_add_phi(p_prev_head, p_new_vreg);
        ir_basic_block_branch_target_add_param(p_prev_head->p_branch->p_target_1, ir_operand_vreg_gen(p_new_vreg));
    }

    list_for_each(p_node, &p_loop_head->varray_basic_block_phis) {
        p_ir_varray_bb_phi p_des_phi = list_entry(p_node, ir_varray_bb_phi, node);
        assert(p_des_phi->p_basic_block == p_loop_head);
        p_ir_varray p_des_varray = p_des_phi->p_varray_phi;
        assert(p_des_varray->varray_def_type == varray_bb_phi_def);
        assert(p_des_varray->p_varray_bb_phi == p_des_phi);
        p_ir_varray p_new_varray = ir_varray_copy(p_des_varray);
        p_ir_varray_bb_phi p_varray_phi = ir_basic_block_add_varray_phi(p_prev_head, p_new_varray);
        ir_basic_block_branch_target_add_varray_param(p_prev_head->p_branch->p_target_1, p_varray_phi, ir_varray_use_gen(p_new_varray));
    }

    for (size_t i = 0; i < target_node_cnt; ++i) {
        p_ir_basic_block_branch_target p_target = target_node_map[i]->p_target;
        ir_branch_target_node_drop(target_node_map[i]);
        p_target->p_block = p_prev_head;
        ir_basic_block_add_prev_target(p_target, p_prev_head);
    }

    if (p_loop_head == p_func->p_entry_block)
        p_func->p_entry_block = p_prev_head;

exit:
    free(target_node_map);
}

void ir_opt_loop_pre_head(p_program p_ir) {
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);

    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        p_list_head p_node;
        list_for_each(p_node, &p_func->p_nestedtree_root->son_list) {
            p_nestedtree_node p_loop = list_entry(p_node, nested_list_node, node)->p_nested_node;
            assert(p_loop->depth);
            _ir_opt_loop_pre_head_loop(p_func, p_loop);
        }
    }

    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
}
