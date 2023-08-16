#include <ir_opt/code_copy.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/gcm.h>
#include <ir_opt/gvn.h>
#include <ir_opt/loop_unrolling.h>
#include <ir_opt/scev.h>
#include <ir_opt/simplify_cfg.h>

#include <ir_gen.h>
#include <ir_print.h>
#include <symbol_gen.h>

p_program get_program;

int key = -1;
p_nestedtree_node find_root = NULL;

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
    ir_simplify_cfg_pass(p_program);
    printf("------ loop full unrolling end ------\n");
}

void ir_opt_loop_unrolling(p_program p_program, int unrolling_time) {
    program_ir_nestree_print(p_program);
    printf("------ loop unrolling begin ------\n");
    bool flag = true;
    get_program = p_program;
    while (unrolling_time-- && flag) {
        flag = false;
        find_root = NULL;
        key = -1;
        p_list_head p_node;
        list_for_each(p_node, &p_program->function) {
            p_symbol_func p_func = list_entry(p_node, symbol_func, node);
            p_list_head p_vreg_node;
            list_for_each(p_vreg_node, &p_func->param_reg_list) {
                list_entry(p_vreg_node, ir_vreg, node)->is_loop_inv = true;
            }
            list_for_each(p_vreg_node, &p_func->vreg_list) {
                list_entry(p_vreg_node, ir_vreg, node)->is_loop_inv = true;
            }
            heap_loop_add(p_func->p_nestedtree_root);
            symbol_func_set_block_id(p_func);
        }
        if (find_root == NULL) break;
        int cost = _loop_instr_cnt(find_root);
        int program_instr_num = _program_instr_cnt(get_program);
        printf("try to unrolling loop: head %ld\n", find_root->head->block_id);
        if (cost < 10 && unrolling_time > 8) {
            if (program_instr_num < 500)
                loop_unrolling(find_root, 2, false);
            else if (program_instr_num < 1000)
                loop_unrolling(find_root, 2, false);
            else if (program_instr_num < 4000)
                loop_unrolling(find_root, 2, false);
            else
                loop_unrolling(find_root, 8, false);
        }
        else if (cost < 30) {
            if (program_instr_num < 500)
                loop_unrolling(find_root, 2, false);
            else if (program_instr_num < 1000)
                loop_unrolling(find_root, 2, false);
            else if (program_instr_num < 4000)
                loop_unrolling(find_root, 2, false);
            else
                loop_unrolling(find_root, 8, false);
        }
        else if (cost < 100) {
            if (program_instr_num < 500)
                loop_unrolling(find_root, 16, false);
            else if (program_instr_num < 1000)
                loop_unrolling(find_root, 8, false);
            else if (program_instr_num < 4000)
                loop_unrolling(find_root, 4, false);
            else
                loop_unrolling(find_root, 2, false);
        }
        else if (cost < 300 && _program_instr_cnt(get_program) < 4000)
            loop_unrolling(find_root, 2, false);
        else if (cost < 500 && _program_instr_cnt(get_program) < 2000)
            loop_unrolling(find_root, 2, false);
        else if (cost < 1000 && _program_instr_cnt(get_program) < 500)
            loop_unrolling(find_root, 2, false);
    }
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        symbol_func_set_block_id(p_func);
    }
    printf("------ loop unrolling end ------\n");
}

void heap_loop_add(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nested_list_node p_son = list_entry(p_node, nested_list_node, node);
        heap_loop_add(p_son->p_nested_node);
    }
    if (!root->head) return;
    invariant_analysis(root);
    int goal = 0;
    list_for_each(p_node, &root->p_var_table) goal = goal + 10;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_block->p_nestree_node != root) continue;
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.p_des->scev_kind != scev_unknown) goal = goal + 1;
        }
    }
    goal = goal * (_program_instr_cnt(get_program) / _loop_instr_cnt(root)) / 5;
    if (goal > key) find_root = root;
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
    int step_cnt = div;
    if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_l_op)
        step_cnt--;
    else if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_g_op)
        step_cnt++;
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

static inline void _prev_loop_unrolling_edge_set(p_nestedtree_node root, int k, p_ir_basic_block p_cond_copy, p_ir_basic_block p_head_copy, p_ir_basic_block p_latch_copy) {
    p_ir_basic_block_branch_target p_target1 = root->p_loop_pre_block->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_target2 = root->p_loop_pre_block->p_branch->p_target_2;
    p_ir_basic_block_branch_target p_prev_target = NULL;
    if (p_target1->p_block == root->head)
        p_prev_target = p_target1;
    else if (p_target2 && p_target2->p_block == root->head)
        p_prev_target = p_target2;
    assert(p_prev_target);
    ir_basic_block_branch_del_prev_target(p_prev_target);
    p_prev_target->p_block = p_head_copy;
    ir_basic_block_add_prev_target(p_prev_target, p_head_copy);
    p_list_head p_node;
    assert(p_cond_copy->p_branch->kind == ir_cond_branch);
    p_prev_target = NULL;
    p_ir_basic_block_branch_target p_latch_target = NULL;
    p_target1 = p_cond_copy->p_branch->p_target_1;
    p_target2 = p_cond_copy->p_branch->p_target_2;
    if (p_target1->p_block == p_latch_copy)
        p_prev_target = p_target2, p_latch_target = p_target1;
    else if (p_target2->p_block == p_latch_copy)
        p_prev_target = p_target1, p_latch_target = p_target2;
    assert(p_prev_target && p_latch_target);
    ir_basic_block_branch_del_prev_target(p_prev_target);
    p_prev_target->p_block = root->head;
    ir_basic_block_add_prev_target(p_prev_target, root->head);
    ir_basic_block_branch_target_clear_param(p_prev_target);

    list_for_each(p_node, &p_latch_target->block_param) {
        p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
        ir_basic_block_branch_target_add_param(p_prev_target, ir_operand_copy(p_param));
    }

    p_ir_operand p1;
    if (root->p_loop_step->p_cond_instr->ir_binary.p_src2->kind == imme) {
        if (root->p_loop_step->p_cond_instr->ir_binary.p_src2->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(root->p_loop_step->p_cond_instr->ir_binary.p_src2->i32const);
        else
            p1 = ir_operand_int_gen(root->p_loop_step->p_cond_instr->ir_binary.p_src2->f32const);
    }

    else
        p1 = ir_operand_vreg_gen(root->p_loop_step->p_cond_instr->ir_binary.p_src2->p_vreg);
    p_ir_operand p2 = ir_operand_vreg_gen(root->p_loop_step->p_basic_var->var_start);
    p_ir_vreg p_count = ir_vreg_copy(p2->p_vreg);
    if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_l_op || root->p_loop_step->p_cond_instr->ir_binary.op == ir_leq_op)
        instr_insert(ir_sub_op, p1, p2, p_count, root->p_loop_pre_block);
    else
        instr_insert(ir_sub_op, p2, p1, p_count, root->p_loop_pre_block);
    if (root->p_loop_step->is_xeq) {
        p1 = ir_operand_vreg_gen(p_count);
        p2 = ir_operand_int_gen(1);
        p_count = ir_vreg_copy(p_count);
        instr_insert(ir_add_op, p1, p2, p_count, root->p_loop_pre_block);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
    }
    p1 = ir_operand_copy(root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2);
    p2 = ir_operand_int_gen(k);
    p_ir_vreg p_count2 = ir_vreg_copy(p_count);
    instr_insert(ir_mul_op, p1, p2, p_count2, root->p_loop_pre_block);

    p1 = ir_operand_vreg_gen(p_count);
    p2 = ir_operand_vreg_gen(p_count2);
    p_count = ir_vreg_copy(p1->p_vreg);
    instr_insert(ir_mod_op, p1, p2, p_count, root->p_loop_pre_block);
    if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_l_op || root->p_loop_step->p_cond_instr->ir_binary.op == ir_leq_op) {
        p1 = ir_operand_vreg_gen(p_count);
        p2 = ir_operand_vreg_gen(root->p_loop_step->p_basic_var->var_start);
        p_count = ir_vreg_copy(p1->p_vreg);
        instr_insert(ir_add_op, p1, p2, p_count, root->p_loop_pre_block);
    }
    else {
        p1 = ir_operand_vreg_gen(p_count);
        p2 = ir_operand_vreg_gen(root->p_loop_step->p_basic_var->var_start);
        p_count = ir_vreg_copy(p1->p_vreg);
        instr_insert(ir_sub_op, p2, p1, p_count, root->p_loop_pre_block);
    }

    p_ir_instr p_instr_copy = NULL;
    list_for_each(p_node, &p_cond_copy->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        if (p_instr->irkind != ir_binary) continue;
        if (!p_instr->ir_binary.p_des->if_cond) continue;
        p_instr_copy = p_instr;
        break;
    }
    assert(p_instr_copy);
    p2 = ir_operand_vreg_gen(p_count);

    ir_instr_reset_binary(p_instr_copy, p_instr_copy->ir_binary.op, p1, p2, p_instr_copy->ir_binary.p_des);
}

void prev_loop_add(p_nestedtree_node root, int k) {
    p_copy_map p_map = ir_code_copy_map_gen();
    p_list_head p_node;
    p_ir_basic_block p_cond_block = NULL;
    list_for_each(p_node, &root->p_loop_latch_block->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        assert(p_target->p_source_block->p_branch->kind == ir_cond_branch);
        assert(p_cond_block == NULL);
        p_cond_block = p_target->p_source_block;
    }
    p_ir_basic_block p_cond_copy, p_head_copy, p_latch_copy;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        loop_block_vreg_copy(p_block, p_map);
        p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_block, p_map);
        if (p_block == p_cond_block)
            p_cond_copy = p_bb_copy;
        if (p_block == root->head)
            p_head_copy = p_bb_copy;
        if (p_block == root->p_loop_latch_block)
            p_latch_copy = p_bb_copy;
        ir_basic_block_insert_next(p_bb_copy, root->head);
    }
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ir_code_copy_instr_of_block(p_block, p_map);
    }
    _prev_loop_unrolling_edge_set(root, k, p_cond_copy, p_head_copy, p_latch_copy);
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
    // if (!is_full)
    //     prev_loop_add(root, k);
    //  return;
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
    p_ir_basic_block p_prev_head = NULL;
    if (!is_full) {
        p_copy_map p_prev_map = ir_code_copy_map_gen();
        p_list_head p_node;
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            loop_block_vreg_copy(p_block, p_prev_map);
            p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_block, p_prev_map);
            if (p_block == root->head)
                p_prev_head = p_bb_copy;
            ir_basic_block_insert_next(p_bb_copy, root->head);
        }
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            ir_code_copy_instr_of_block(p_block, p_prev_map);
        }
        assert(p_prev_head);
        ir_code_copy_map_drop(p_prev_map);
    }
    for (int i = k - 1; i >= 1; --i) {
        _loop_unrolling_edge_set(root, p_head_copy[i], p_cond_block, p_latch_copy[i]);
        p_cond_block = p_cond_copy[i];
        ir_code_copy_map_drop(p_map[i]);
    }
    if (is_full) {
        p_ir_basic_block_branch_target p_target1 = p_cond_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target2 = p_cond_block->p_branch->p_target_2;
        ir_operand_drop(p_cond_block->p_branch->p_exp);
        p_cond_block->p_branch->p_exp = NULL;
        p_cond_block->p_branch->kind = ir_br_branch;
        if (p_target1->p_block == root->p_loop_latch_block) {
            ir_basic_block_branch_del_prev_target(p_target1);
            ir_basic_block_branch_target_drop(p_cond_block, p_target1);
            p_cond_block->p_branch->p_target_1 = p_target2;
        }
        else {
            ir_basic_block_branch_del_prev_target(p_target2);
            ir_basic_block_branch_target_drop(p_cond_block, p_target2);
        }
        p_cond_block->p_branch->p_target_2 = NULL;
        assert(list_head_alone(&root->p_loop_latch_block->prev_branch_target_list));
    }
    else {
        p_list_head p_node;
        p_ir_instr p_cond_instr = NULL;
        list_for_each(p_node, &p_cond_block->p_branch->p_exp->p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.p_des != p_cond_block->p_branch->p_exp->p_vreg) continue;
            p_cond_instr = p_instr->ir_binary.p_src1->p_vreg->p_instr_def;
            break;
        }
        assert(p_cond_instr);
        ir_instr_print(p_cond_instr);
        p_ir_operand p1, p2;
        p_ir_vreg p_des;
        if (check_operand(p_cond_instr->ir_binary.p_src1)) {
            p1 = ir_operand_copy(p_cond_instr->ir_binary.p_src1);
            p2 = ir_operand_int_gen(k);
            p_des = ir_vreg_copy(p_cond_instr->ir_binary.p_des);
            instr_insert(ir_mul_op, p1, p2, p_des, root->p_loop_pre_block);
            p1 = ir_operand_vreg_gen(p_des);
            p2 = ir_operand_copy(root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2);
            ir_instr_reset_binary(p_cond_instr, p_cond_instr->ir_binary.op, p1, p2, p_cond_instr->ir_binary.p_des);
        }
        else {
            p1 = ir_operand_copy(p_cond_instr->ir_binary.p_src2);
            p2 = ir_operand_int_gen(k);
            p_des = ir_vreg_copy(p_cond_instr->ir_binary.p_des);
            instr_insert(ir_mul_op, p1, p2, p_des, root->p_loop_pre_block);
            p1 = ir_operand_copy(root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src1);
            p2 = ir_operand_vreg_gen(p_des);
            ir_instr_reset_binary(p_cond_instr, p_cond_instr->ir_binary.op, p1, p2, p_cond_instr->ir_binary.p_des);
        }

        p_ir_basic_block_branch_target p_target1 = root->p_loop_pre_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target2 = root->p_loop_pre_block->p_branch->p_target_2;
        p_ir_basic_block_branch_target p_exit_target = NULL, p_latch_target = NULL, p_entry_target = NULL;
        if (p_target1->p_block == root->head) p_entry_target = p_target1;
        else
            p_entry_target = p_target2;

        p_ir_basic_block p_new_block = ir_basic_block_gen();
        ir_basic_block_branch_del_prev_target(p_entry_target);
        p_entry_target->p_block = p_new_block;
        ir_basic_block_add_prev_target(p_entry_target, p_new_block);
        ir_basic_block_insert_prev(p_new_block, root->head);
        p1 = ir_operand_vreg_gen(root->p_loop_step->p_basic_var->var_start);
        p2 = ir_operand_vreg_gen(p_des);
        p_ir_vreg p_new_vreg = ir_vreg_copy(p_des);
        p_new_vreg->if_cond = true;
        instr_insert(p_cond_instr->ir_binary.op, p1, p2, p_new_vreg, p_new_block);
        p1 = ir_operand_vreg_gen(p_new_vreg);
        p2 = ir_operand_copy(root->p_loop_step->p_cond_instr->ir_binary.p_src2);
        p_ir_vreg p_new_vreg1 = ir_vreg_copy(p_new_vreg);

        instr_insert(root->p_loop_step->p_cond_instr->ir_binary.op, p1, p2, p_new_vreg1, p_new_block);
        p1 = ir_operand_vreg_gen(p_new_vreg1);
        ir_basic_block_set_cond(p_new_block, p1, root->head, p_prev_head);
        list_for_each(p_node, &p_entry_target->block_param) {
            p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
            p_ir_vreg p_vreg = ir_vreg_copy(p_param->p_vreg);
            symbol_func_vreg_add(root->head->p_func, p_vreg);
            ir_basic_block_add_phi(p_new_block, p_vreg);
            ir_basic_block_branch_target_add_param(p_new_block->p_branch->p_target_1, ir_operand_vreg_gen(p_vreg));
            ir_basic_block_branch_target_add_param(p_new_block->p_branch->p_target_2, ir_operand_vreg_gen(p_vreg));
        }
        p_ir_operand p_save = ir_operand_copy(p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.p_src2);
        p1 = ir_operand_copy(p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.p_src2);
        p2 = ir_operand_vreg_gen(p_des);
        p_new_vreg1 = ir_vreg_copy(p_des);
        // if (p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.op == ir_l_op || p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.op == ir_leq_op)
        instr_insert(ir_sub_op, p1, p2, p_new_vreg1, p_cond_block);
        // else
        //    instr_insert(ir_add_op, p1, p2, p_new_vreg1, p_cond_block);
        p1 = ir_operand_copy(p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.p_src1);
        p2 = ir_operand_vreg_gen(p_new_vreg1);
        ir_instr_reset_binary(p_cond_block->p_branch->p_exp->p_vreg->p_instr_def, p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.op, p1, p2, p_cond_block->p_branch->p_exp->p_vreg);

        p_target1 = p_cond_block->p_branch->p_target_1;
        p_target2 = p_cond_block->p_branch->p_target_2;
        p_exit_target = NULL, p_latch_target = NULL;
        if (p_target1->p_block == root->p_loop_latch_block)
            p_exit_target = p_target2, p_latch_target = p_target1;
        else
            p_exit_target = p_target1, p_latch_target = p_target2;
        p_new_block = ir_basic_block_gen();
        ir_basic_block_insert_prev(p_new_block, p_prev_head);
        p1 = ir_operand_vreg_gen(p_des);
        ir_basic_block_set_cond(p_new_block, p1, p_prev_head, p_exit_target->p_block);

        ir_basic_block_branch_del_prev_target(p_exit_target);
        p_exit_target->p_block = p_new_block;
        ir_basic_block_add_prev_target(p_exit_target, p_new_block);
        list_for_each(p_node, &p_exit_target->block_param) {
            p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
            ir_basic_block_branch_target_add_param(p_new_block->p_branch->p_target_2, ir_operand_copy(p_param));
        }
        ir_basic_block_branch_target_clear_param(p_exit_target);

        list_for_each(p_node, &p_latch_target->block_param) {
            p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
            p_ir_operand p_operand_copy = ir_operand_copy(p_param);
            ir_basic_block_branch_target_add_param(p_exit_target, p_operand_copy);
            p_ir_vreg p_new_vreg = ir_vreg_copy(p_operand_copy->p_vreg);
            symbol_func_vreg_add(root->head->p_func, p_new_vreg);
            ir_basic_block_add_phi(p_new_block, p_new_vreg);
            if (p_param->p_vreg == p_cond_instr->ir_binary.p_des) {
                p1 = ir_operand_vreg_gen(p_param->p_vreg);

                p_ir_vreg p_des = ir_vreg_copy(p_param->p_vreg);
                p_des->if_cond = true;
                instr_insert(p_cond_block->p_branch->p_exp->p_vreg->p_instr_def->ir_binary.op, p1, p_save, p_des, p_new_block);
                ir_operand_drop(p_new_block->p_branch->p_exp);
                ir_basic_block_set_cond_exp(p_new_block, ir_operand_vreg_gen(p_des));
            }
            p_operand_copy = ir_operand_vreg_gen(p_new_vreg);
            ir_basic_block_branch_target_add_param(p_new_block->p_branch->p_target_1, p_operand_copy);
        }
    }
}
