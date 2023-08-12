#include <ir_opt/loop_unswitch.h>

#include <ir_opt/code_copy.h>

#include <ir_gen.h>
#include <ir_opt/gcm.h>
#include <ir_opt/simplify_cfg.h>
#include <ir_print.h>
#include <symbol_gen.h>
#include <symbol_gen/func.h>

const size_t MAX_INSTR_NUM = 6000;

size_t program_instr_cnt = 0;

void ir_opt_loop_unswitch(p_program p_program) {
    printf("\n loop unswitch begin\n");
    program_instr_cnt = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        program_instr_cnt += p_func->instr_num;
    }

    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        p_list_head p_son_node;
        list_for_each(p_son_node, &p_func->p_nestedtree_root->son_list) {
            p_nestedtree_node p_list_node = list_entry(p_son_node, nested_list_node, node)->p_nested_node;
            loop_unswitch_try(p_list_node);
        }
    }
    ir_simplify_cfg_pass(p_program);
    ir_opt_gcm(p_program);
    printf("\n loop unswitch end\n");
}

static inline size_t _unswitch_cost_cal(p_nestedtree_node root, p_ir_basic_block switch_block) {
    p_ir_basic_block p_block1 = switch_block->p_branch->p_target_1->p_block;
    p_ir_basic_block p_block2 = switch_block->p_branch->p_target_2->p_block;
    if (!ir_basic_block_dom_check(p_block1, switch_block)) p_block1 = NULL;
    if (!ir_basic_block_dom_check(p_block2, switch_block)) p_block2 = NULL;
    size_t ret = 0;
    p_list_head p_node;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        ret += p_block->instr_num;
        if (p_block1 && ir_basic_block_dom_check(p_block, p_block1)) ret -= p_block->instr_num;
        if (p_block2 && ir_basic_block_dom_check(p_block, p_block2)) ret -= p_block->instr_num;
    }
    return ret;
}

void loop_unswitch_try(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nestedtree_node p_list_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        loop_unswitch_try(p_list_node);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_list_node->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block_list_node, node)->p_basic_block;
            if (!search(root->rbtree->root, (uint64_t) p_block)) {
                p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
                *p_new_node = (ir_basic_block_list_node) {
                    .p_basic_block = p_block,
                    .node = list_init_head(&p_new_node->node),
                };
                list_add_next(&p_new_node->node, &root->head->loop_node_list);
                insert(root->rbtree, (uint64_t) p_block);
            }
        }
    }

    if (root->head->p_branch->kind == ir_cond_branch) {
        if (search(root->rbtree->root, (uint64_t) root->head->p_branch->p_target_1->p_block) && search(root->rbtree->root, (uint64_t) root->head->p_branch->p_target_2->p_block)) {
            p_ir_instr p_cond_instr = root->head->p_branch->p_exp->p_vreg->p_instr_def;
            if (check_operand(p_cond_instr->ir_binary.p_src1) && check_operand(p_cond_instr->ir_binary.p_src2)) {
                ir_instr_print(p_cond_instr);
                size_t cnt = _unswitch_cost_cal(root, root->head);
                if ((cnt <= 88 && program_instr_cnt + cnt <= MAX_INSTR_NUM) || (cnt < 300 && program_instr_cnt + cnt <= MAX_INSTR_NUM / 2)) {
                    loop_unswitch(root, root->head);
                    program_instr_cnt += cnt;
                }
                return;
            }
        }
    }

    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_block->p_branch->kind == ir_cond_branch) {
            if (search(root->rbtree->root, (uint64_t) p_block->p_branch->p_target_1->p_block) && search(root->rbtree->root, (uint64_t) p_block->p_branch->p_target_2->p_block)) {
                p_ir_instr p_cond_instr = p_block->p_branch->p_exp->p_vreg->p_instr_def;
                if (check_operand(p_cond_instr->ir_binary.p_src1) && check_operand(p_cond_instr->ir_binary.p_src2)) {
                    ir_instr_print(p_cond_instr);
                    size_t cnt = _unswitch_cost_cal(root, p_block);
                    if ((cnt <= 88 && program_instr_cnt + cnt <= MAX_INSTR_NUM) || (cnt < 300 && program_instr_cnt + cnt <= MAX_INSTR_NUM / 2)) {
                        loop_unswitch(root, p_block);
                        program_instr_cnt += cnt;
                    }
                    return;
                }
            }
        }
    }
}

static inline void _unswitch_edge_set(p_nestedtree_node root, p_ir_basic_block p_switch_block, p_ir_basic_block p_switch_copy, p_ir_basic_block p_contrl_block) {
    ir_basic_block_branch_del_prev_target(root->p_loop_pre_block->p_branch->p_target_1);
    root->p_loop_pre_block->p_branch->p_target_1->p_block = p_contrl_block;
    ir_basic_block_add_prev_target(root->p_loop_pre_block->p_branch->p_target_1, p_contrl_block);

    p_ir_basic_block_branch_target p_target2 = p_switch_block->p_branch->p_target_2;
    ir_basic_block_branch_del_prev_target(p_target2);
    ir_operand_drop(p_switch_block->p_branch->p_exp);
    p_switch_block->p_branch->p_exp = NULL;
    p_switch_block->p_branch->kind = ir_br_branch;
    ir_basic_block_branch_target_drop(p_switch_block, p_target2);
    p_switch_block->p_branch->p_target_2 = NULL;

    p_ir_basic_block_branch_target p_target1 = p_switch_copy->p_branch->p_target_1;
    ir_basic_block_branch_del_prev_target(p_target1);
    ir_operand_drop(p_switch_copy->p_branch->p_exp);
    p_switch_copy->p_branch->p_exp = NULL;
    p_switch_copy->p_branch->kind = ir_br_branch;
    ir_basic_block_branch_target_drop(p_switch_copy, p_target1);
    p_switch_copy->p_branch->p_target_1 = p_switch_copy->p_branch->p_target_2;
    p_switch_copy->p_branch->p_target_2 = NULL;
}

void loop_unswitch(p_nestedtree_node root, p_ir_basic_block p_switch_block) {
    assert(p_switch_block->p_branch->kind == ir_cond_branch);
    assert(search(root->rbtree->root, (uint64_t) p_switch_block->p_branch->p_target_1->p_block));
    assert(search(root->rbtree->root, (uint64_t) p_switch_block->p_branch->p_target_2->p_block));
    p_copy_map p_map = ir_code_copy_map_gen();
    p_list_head p_node;
    list_head add_list = list_init_head(&add_list);
    p_ir_basic_block p_switch_copy = NULL;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_block == root->head) continue;
        loop_block_vreg_copy(p_block, p_map);
        p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_block, p_map);
        if (p_block == p_switch_block) p_switch_copy = p_bb_copy;
        ir_basic_block_insert_next(p_bb_copy, root->head);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_bb_copy,
            .node = list_init_head(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &add_list);
        insert(root->rbtree, (uint64_t) p_bb_copy);
    }
    loop_block_vreg_copy(root->head, p_map);
    p_ir_basic_block p_head_copy = ir_code_copy_bb(root->head, p_map);
    ir_basic_block_insert_next(p_head_copy, root->head);
    ir_code_copy_instr_of_block(root->head, p_map);
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = p_head_copy,
        .node = list_init_head(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &add_list);
    insert(root->rbtree, (uint64_t) p_head_copy);
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_block == root->head) continue;
        ir_code_copy_instr_of_block(p_block, p_map);
    }
    if (p_switch_block == root->head)
        p_switch_copy = p_head_copy;
    p_ir_basic_block p_contrl_block = ir_basic_block_gen();
    ir_basic_block_insert_prev(p_contrl_block, root->head);
    p_ir_instr p_cond_instr = p_switch_block->p_branch->p_exp->p_vreg->p_instr_def;
    p_ir_operand p1 = ir_operand_copy(p_cond_instr->ir_binary.p_src1);
    p_ir_operand p2 = ir_operand_copy(p_cond_instr->ir_binary.p_src2);
    p_ir_vreg p_vreg = ir_vreg_copy(p_cond_instr->ir_binary.p_des);
    symbol_func_vreg_add(root->head->p_func, p_vreg);
    p_ir_instr p_instr = ir_binary_instr_gen(p_cond_instr->ir_binary.op, p1, p2, p_vreg);
    ir_basic_block_addinstr_tail(p_contrl_block, p_instr);
    p_ir_operand p_cond = ir_operand_vreg_gen(p_vreg);
    ir_basic_block_set_cond(p_contrl_block, p_cond, root->head, p_head_copy);

    p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = p_contrl_block,
        .node = list_init_head(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &add_list);

    list_for_each(p_node, &root->head->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_new_vreg = ir_vreg_copy(p_vreg);
        symbol_func_vreg_add(root->head->p_func, p_new_vreg);
        ir_basic_block_add_phi(p_contrl_block, p_new_vreg);
        ir_basic_block_branch_target_add_param(p_contrl_block->p_branch->p_target_1, ir_operand_vreg_gen(p_new_vreg));
        ir_basic_block_branch_target_add_param(p_contrl_block->p_branch->p_target_2, ir_operand_vreg_gen(p_new_vreg));
    }

    _unswitch_edge_set(root, p_switch_block, p_switch_copy, p_contrl_block);

    symbol_func_set_block_id(root->head->p_func);
    while (!list_head_alone(&add_list)) {
        p_node = (&add_list)->p_next;
        list_del(p_node);
        list_add_next(p_node, &root->head->loop_node_list);
    }
    ir_code_copy_map_drop(p_map);
}