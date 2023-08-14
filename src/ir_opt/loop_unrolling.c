#include <ir_opt/code_copy.h>
#include <ir_opt/loop_unrolling.h>
#include <ir_opt/scev.h>

#include <ir_gen.h>
#include <ir_print.h>
#include <symbol_gen.h>

p_program get_program;

void ir_opt_loop_full_unrolling(p_program p_program) {
    program_ir_nestree_print(p_program);
    printf("------ loop full unrolling begin ------\n");
    get_program = p_program;
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        loop_full_unrolling(p_func->p_nestedtree_root);
        symbol_func_set_block_id(p_func);
    }

    printf("------ loop full unrolling end ------\n");
}

static inline int _program_instr_cnt(p_program p_program) {
    int ret = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        ret += p_func->instr_num;
    }
    return ret;
}

static inline int _loop_instr_cnt(p_nestedtree_node root) {
    int ret = 0;
    p_list_head p_node;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ret += p_block->instr_num;
    }
    return ret;
}

void loop_full_unrolling(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nested_list_node p_tail = list_entry(p_node, nested_list_node, node);
        loop_full_unrolling(p_tail->p_nested_node);
    }
    if (!list_head_alone(&root->son_list)) return;
    if (root->p_loop_step == NULL) return;
    if (root->p_loop_step->p_cond_instr->ir_binary.p_src2->kind != imme) return;
    if (root->p_loop_step->p_cond_instr->ir_binary.p_src2->p_type->basic != type_i32 || root->p_loop_step->p_cond_instr->ir_binary.p_src2->i32const > 100) return;
    if (root->p_loop_step->p_basic_var->basic_var->p_type->basic != type_i32) return;
    int div = 0;
    if (root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1->kind == reg) {
        if (root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1->p_vreg == root->p_loop_step->p_basic_var->basic_var) {
            if (root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2->kind != imme && root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2->p_type->basic != type_i32)
                return;
            else
                div = root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2->i32const;
        }
    }
    else if (root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2->p_vreg == root->p_loop_step->p_basic_var->basic_var) {
        if (root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1->kind != imme && root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1->p_type->basic != type_i32)
            return;
        else
            div = root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1->i32const;
    }
    assert(div);
    // p_ir_vreg p_vreg = root->p_loop_step->p_basic_var->basic_var;
    if (root->p_loop_step->p_basic_var->var_start->p_instr_def->irkind != ir_unary) return;
    if (root->p_loop_step->p_basic_var->var_start->p_instr_def->ir_unary.op != ir_val_assign) return;
    if (root->p_loop_step->p_basic_var->var_start->p_instr_def->ir_unary.p_src->kind != imme) return;
    if (root->p_loop_step->p_basic_var->var_start->p_instr_def->ir_unary.p_src->p_type->basic != type_i32) return;
    int step_cnt = 0;
    if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_leq_op)
        step_cnt = 1;
    else if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_geq_op)
        step_cnt = -1;
    step_cnt = (step_cnt + root->p_loop_step->p_cond_instr->ir_binary.p_src2->i32const - root->p_loop_step->p_basic_var->var_start->p_instr_def->ir_unary.p_src->i32const) / div;
    if (step_cnt > 64) return;
    int add_instr = _loop_instr_cnt(root) * (step_cnt - 1);
    int sum_instr = _program_instr_cnt(get_program);
    if (add_instr > 600 && sum_instr > 1000) return;
    if (add_instr > 300 && sum_instr > 2000) return;
    if (add_instr + sum_instr > 7000) return;
    printf("loop full unrolling %ld: step = %d, add instr num = %d\n", root->head->block_id, step_cnt, add_instr);
    loop_unrolling(root, step_cnt, true);
}

void prev_loop_add(p_nestedtree_node root, int k) {
    p_copy_map p_map = ir_code_copy_map_gen();
    p_list_head p_node;
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        loop_block_vreg_copy(p_block, p_map);
        p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_block, p_map);
        ir_basic_block_insert_next(p_bb_copy, root->head);
    }
    loop_block_vreg_copy(root->p_loop_latch_block, p_map);
    p_ir_basic_block p_latch_copy = ir_code_copy_bb(root->p_loop_latch_block, p_map);
    ir_basic_block_insert_next(p_latch_copy, root->head);
    loop_block_vreg_copy(root->head, p_map);

    p_ir_basic_block p_head_copy = ir_code_copy_bb(root->head, p_map);
    ir_basic_block_insert_next(p_head_copy, root->head);
    ir_code_copy_instr_of_block(root->head, p_map);
    ir_code_copy_instr_of_block(root->p_loop_latch_block, p_map);
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ir_code_copy_instr_of_block(p_block, p_map);
    }
    // prev_loop_unrolling_edge_set(root, k, p_map);
    ir_code_copy_map_drop(p_map);
}

static inline void _loop_unrolling_edge_set(p_nestedtree_node root, p_ir_basic_block p_head_copy, p_ir_basic_block p_cond_copy, p_ir_basic_block p_latch_copy) {
    p_ir_basic_block_branch_target p_target1 = p_cond_copy->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_target2 = p_cond_copy->p_branch->p_target_2;
    ir_basic_block_branch_del_prev_target(p_target1);
    ir_basic_block_branch_del_prev_target(p_target2);
    ir_operand_drop(p_cond_copy->p_branch->p_exp);
    p_cond_copy->p_branch->p_exp = NULL;
    p_cond_copy->p_branch->kind = ir_br_branch;
    if (p_target1->p_block == root->p_loop_latch_block)
        ir_basic_block_branch_target_drop(p_cond_copy, p_target2);
    else {
        ir_basic_block_branch_target_drop(p_cond_copy, p_target1);
        p_cond_copy->p_branch->p_target_1 = p_target2;
    }
    p_cond_copy->p_branch->p_target_1->p_block = p_head_copy;
    ir_basic_block_add_prev_target(p_cond_copy->p_branch->p_target_1, p_head_copy);
    p_cond_copy->p_branch->p_target_2 = NULL;

    ir_basic_block_branch_del_prev_target(p_latch_copy->p_branch->p_target_1);
    p_latch_copy->p_branch->p_target_1->p_block = root->head;
    ir_basic_block_add_prev_target(p_latch_copy->p_branch->p_target_1, root->head);
    ir_basic_block_branch_del_prev_target(root->p_loop_latch_block->p_branch->p_target_1);
    root->p_loop_latch_block->p_branch->p_target_1->p_block = p_head_copy;
    ir_basic_block_add_prev_target(root->p_loop_latch_block->p_branch->p_target_1, p_head_copy);
    root->p_loop_latch_block = p_latch_copy;
}

void loop_unrolling(p_nestedtree_node root, int k, bool is_full) {
    if (root->p_loop_step == NULL)
        return;
    if (!is_full)
        prev_loop_add(root, k);
    // return;
    p_copy_map p_map[k];
    p_ir_basic_block p_latch_copy[k], p_cond_copy[k], p_head_copy[k];
    for (int i = 1; i < k; ++i) {
        p_map[i] = ir_code_copy_map_gen();
        p_list_head p_node;
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            loop_block_vreg_copy(p_block, p_map[i]);
            p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_block, p_map[i]);
            if (p_block == root->p_loop_latch_block)
                p_latch_copy[i] = p_bb_copy;
            if (p_block == root->p_loop_step->p_cond_instr->p_basic_block)
                p_cond_copy[i] = p_bb_copy;
            if (p_block == root->head)
                p_head_copy[i] = p_bb_copy;
            ir_basic_block_insert_next(p_bb_copy, root->head);
        }
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            ir_code_copy_instr_of_block(p_block, p_map[i]);
        }
    }
    p_ir_basic_block p_cond_block = root->p_loop_step->p_cond_instr->p_basic_block;
    for (int i = k - 1; i >= 1; --i) {
        _loop_unrolling_edge_set(root, p_head_copy[i], p_cond_block, p_latch_copy[i]);
        p_cond_block = p_cond_copy[i];
        ir_code_copy_map_drop(p_map[i]);
    }
}
