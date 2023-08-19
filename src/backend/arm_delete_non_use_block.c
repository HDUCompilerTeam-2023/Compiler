#include <backend/arm/arm_struct_gen.h>
#include <backend/arm/arm_struct_output.h>
#include <program/def.h>
#include <stdio.h>
static inline void arm_delete_empty_block(p_arm_block p_a_block) {
    if (!list_head_alone(&p_a_block->instr_list)) return;
    assert(p_a_block->p_target1);
    assert(!p_a_block->p_target2);

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_a_block->arm_block_prevs) {
        p_arm_instr p_jump = list_entry(p_node, arm_block_edge_node, node)->p_jump_instr;
        arm_jump_instr_reset_target(p_jump, p_a_block->p_target1->jump_instr.p_block_target);
    }
    arm_block_drop(p_a_block);
}
static inline void arm_delete_little_instr(p_arm_block p_a_block) {
    if (p_a_block->p_target2)
        return;
    if (!p_a_block->p_target1)
        return;
    if (list_head_alone(&p_a_block->arm_block_prevs))
        return;
    if (p_a_block->p_target1->cond_type == arm_al)
        return;
    p_list_head p_node;
    size_t instr_num = 0;
    list_for_each(p_node, &p_a_block->instr_list) {
        p_arm_instr p_a_instr = list_entry(p_node, arm_instr, node);
        if (p_a_instr->type == arm_call_type || p_a_instr->type == arm_push_pop_type || p_a_instr->type == arm_vpush_vpop_type) {
            return;
        }
        instr_num++;
    }
    if (instr_num < 5) {
        p_list_head p_next;
        list_for_each_safe(p_node, p_next, &p_a_block->arm_block_prevs) {
            p_arm_instr p_prev_jump = list_entry(p_node, arm_block_edge_node, node)->p_jump_instr;
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_a_block->instr_list) {
                p_arm_instr p_a_instr = list_entry(p_node, arm_instr, node);
                p_arm_instr p_new_instr = arm_instr_copy(p_a_instr);
                p_new_instr->cond_type = p_prev_jump->cond_type;
                arm_block_add_instr_tail(p_prev_jump->jump_instr.p_source_block, p_new_instr);
            }
            if (p_prev_jump->jump_instr.p_source_block->p_target1 == p_prev_jump) {
                arm_block_del_prev(p_prev_jump);
                p_arm_instr p_new_jump = arm_instr_copy(p_a_block->p_target1);
                p_new_jump->jump_instr.p_source_block = p_prev_jump->jump_instr.p_source_block;
                arm_block_set_br1(p_prev_jump->jump_instr.p_source_block, p_new_jump);
                arm_instr_drop(p_prev_jump);
                continue;
            }
            if (p_prev_jump->jump_instr.p_source_block->p_target2 == p_prev_jump) {
                arm_block_del_prev(p_prev_jump);
                p_arm_instr p_new_jump = arm_instr_copy(p_a_block->p_target1);
                p_new_jump->jump_instr.p_source_block = p_prev_jump->jump_instr.p_source_block;
                arm_block_set_br2(p_prev_jump->jump_instr.p_source_block, p_new_jump);
                arm_instr_drop(p_prev_jump);
            }
        }
        arm_block_del_prev(p_a_block->p_target1);
        arm_block_drop(p_a_block);
    }
}
static inline void arm_delete_non_use_block_func(p_arm_func p_func) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_func->block_list) {
        p_arm_block p_a_block = list_entry(p_node, arm_block, node);
        arm_delete_little_instr(p_a_block);
    }
    list_for_each_safe(p_node, p_next, &p_func->block_list) {
        p_arm_block p_a_block = list_entry(p_node, arm_block, node);
        arm_delete_empty_block(p_a_block);
    }
    list_for_each_safe(p_node, p_next, &p_func->block_list) {
        p_arm_block p_a_block = list_entry(p_node, arm_block, node);
        if (!p_a_block->p_target1) continue;
        if (!p_a_block->p_target2) continue;
        if (p_a_block->p_target1->jump_instr.p_block_target == p_a_block->p_target2->jump_instr.p_block_target) {
            assert(p_a_block == p_a_block->p_target1->jump_instr.p_source_block);
            assert(p_a_block == p_a_block->p_target2->jump_instr.p_source_block);
            p_arm_instr p_jump = arm_jump_instr_gen(arm_b, arm_al, p_a_block, p_a_block->p_target1->jump_instr.p_block_target);
            arm_block_del_prev(p_a_block->p_target1);
            arm_block_del_prev(p_a_block->p_target2);
            arm_instr_drop(p_a_block->p_target1);
            arm_instr_drop(p_a_block->p_target2);
            arm_block_set_br1(p_a_block, p_jump);
            p_a_block->p_target2 = NULL;
        }
    }
}

void arm_delete_non_use_block(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->arm_function) {
        p_arm_func p_a_func = list_entry(p_node, arm_func, node);
        arm_delete_non_use_block_func(p_a_func);
    }
}