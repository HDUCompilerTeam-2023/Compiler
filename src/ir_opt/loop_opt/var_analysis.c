#include <util.h>

#include <ir.h>
#include <ir/basic_block.h>
#include <stdio.h>

#include <ir/bb_param.h>
#include <ir/instr.h>
#include <ir/operand.h>
#include <ir/param.h>
#include <ir/vreg.h>
#include <symbol/func.h>
#include <symbol/var.h>

#include <ir_print.h>

#include <symbol/type.h>

#include <ir_opt/loop_opt/var_analysis.h>

void program_var_analysis(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        // printf("\n%s\n", p_func->name);
        loop_var_analysis(p_func->p_nestedtree_root);
    }
}

void loop_var_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nestedtree_node p_list_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        loop_var_analysis(p_list_node);
    }
    // invariant analysis
    invariant_analysis(root);
    step_var_analysis(root);
    basic_var_analysis(root);
}

bool check_operand(p_ir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        if (p_operand->p_type->ref_level > 0) {
            return 0;
        }
        return 1;
    case reg:
        if (p_operand->p_vreg->if_loop_inv)
            return 1;
        return 0;
    }
}

void invariant_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    if (!root->head) return;
    list_for_each(p_node, &root->head->basic_block_phis) {
        list_entry(p_node, ir_bb_phi, node)->p_bb_phi->if_loop_inv = false;
    }
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_binary:
            if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                p_instr->ir_binary.p_des->if_loop_inv = false;
            break;
        case ir_unary:
            if (!check_operand(p_instr->ir_unary.p_src))
                p_instr->ir_unary.p_des->if_loop_inv = false;
            break;
        case ir_load:
            p_instr->ir_load.p_des->if_loop_inv = false;
            break;
        case ir_gep:
            p_instr->ir_gep.p_des->if_loop_inv = false;
            break;
        case ir_call:
            if (p_instr->ir_call.p_func->ret_type != type_void)
                p_instr->ir_call.p_des->if_loop_inv = false;
            break;
        default:
            break;
        }
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_basic_block->basic_block_phis) {
            list_entry(p_list_node, ir_bb_phi, node)->p_bb_phi->if_loop_inv = false;
        }
        list_for_each(p_list_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_binary:
                if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                    p_instr->ir_binary.p_des->if_loop_inv = false;
                break;
            case ir_unary:
                if (!check_operand(p_instr->ir_unary.p_src))
                    p_instr->ir_unary.p_des->if_loop_inv = false;
                break;
            case ir_load:
                p_instr->ir_load.p_des->if_loop_inv = false;
                break;
            case ir_gep:
                p_instr->ir_gep.p_des->if_loop_inv = false;
                break;
            case ir_call:
                if (p_instr->ir_call.p_func->ret_type != type_void)
                    p_instr->ir_call.p_des->if_loop_inv = false;
                break;
            default:
                break;
            }
        }
    }
    return;
    printf("head %ld\n", root->head->block_id);
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_binary:
            if (p_instr->ir_binary.p_des->if_loop_inv)
                printf("%ld\n", p_instr->ir_binary.p_des->id);
            break;
        case ir_unary:
            if (p_instr->ir_unary.p_des->if_loop_inv)
                printf("%ld\n", p_instr->ir_unary.p_des->id);
        default:
            break;
        }
    }
    if (!list_head_alone(&root->tail_list)) {
        list_for_each(p_node, &root->tail_list) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_list_head p_list_node;
            list_for_each(p_list_node, &p_basic_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
                switch (p_instr->irkind) {
                case ir_binary:
                    if (p_instr->ir_binary.p_des->if_loop_inv)
                        printf("%ld\n", p_instr->ir_binary.p_des->id);
                    break;
                case ir_unary:
                    if (p_instr->ir_unary.p_des->if_loop_inv)
                        printf("%ld\n", p_instr->ir_unary.p_des->id);
                default:
                    break;
                }
            }
        }
    }
}

void step_var_analysis(p_nestedtree_node root) {
    if (!root->head) return;
    root->p_info_table = malloc(sizeof(*root->p_info_table));
    *root->p_info_table = (var_info_table) {
        .step_var = NULL,
        .start_var = NULL,
        .p_cond_instr = NULL,
        .ind_var_list = list_head_init(&root->p_info_table->ind_var_list),
    };
    // printf("head %ld\n", root->head->block_id);
    p_ir_basic_block p_exit_block;
    if (!list_head_alone(&root->tail_list)) {
        p_list_head p_node;
        list_for_each(p_node, &root->tail_list) {
            p_exit_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            if (p_exit_block->p_branch->p_target_1->p_block == root->head || p_exit_block->p_branch->p_target_2->p_block == root->head) break;
        }
    }
    else
        p_exit_block = root->head;
    root->p_info_table->p_cond_instr = list_entry(p_exit_block->instr_list.p_prev, ir_instr, node);
    if (root->p_info_table->p_cond_instr->irkind != ir_binary || root->p_info_table->p_cond_instr->ir_binary.op != ir_l_op) {
        free(root->p_info_table);
        root->p_info_table = NULL;
        return;
    }
    p_ir_basic_block_branch_target p_true_block = p_exit_block->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_false_block = p_exit_block->p_branch->p_target_2;
    size_t place = 0;
    assert(p_true_block);
    if (p_true_block->p_block == root->head) {
        p_list_head p_node;
        list_for_each(p_node, &p_true_block->block_param) {
            ++place;
            p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
            if (p_param->p_vreg == root->p_info_table->p_cond_instr->ir_binary.p_src1->p_vreg) break;
        }
    }
    else {
        assert(p_false_block);
        p_list_head p_node;
        list_for_each(p_node, &p_false_block->block_param) {
            ++place;
            p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
            if (p_param->p_vreg == root->p_info_table->p_cond_instr->ir_binary.p_src1->p_vreg) break;
        }
    }
    p_list_head p_node;
    size_t tmp = place;
    list_for_each(p_node, &root->head->basic_block_phis) {
        if (--tmp) continue;
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        root->p_info_table->step_var = p_phi->p_bb_phi;
    }
    p_ir_basic_block_branch_target p_target;
    list_for_each(p_node, &root->head->prev_branch_target_list) {
        p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        if (!search(root->rbtree->root, (uint64_t) p_target->p_source_block)) break;
    }
    list_for_each(p_node, &p_target->block_param) {
        if (--place) continue;
        p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
        root->p_info_table->start_var = p_param->p_vreg;
    }

    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        if (p_instr->irkind != ir_binary) continue;
        if (p_instr->ir_binary.p_des == root->p_info_table->p_cond_instr->ir_binary.p_src1->p_vreg) {
            if (p_instr->ir_binary.p_src1->kind == reg) {
                if (p_instr->ir_binary.p_src1->p_vreg == root->p_info_table->step_var) {
                    if (check_operand(p_instr->ir_binary.p_src2)) {
                        root->is_simple_loop = true;
                        root->p_info_table->p_step_instr = p_instr;
                        // ir_vreg_print(root->p_info_table->start_var);
                        // putchar(10);
                        // ir_instr_print(root->p_info_table->p_step_instr);
                        // ir_instr_print(root->p_info_table->p_cond_instr);
                        return;
                    }
                }
            }
            else if (p_instr->ir_binary.p_src2->kind == reg) {
                if (p_instr->ir_binary.p_src2->p_vreg == root->p_info_table->step_var) {
                    if (check_operand(p_instr->ir_binary.p_src1)) {
                        root->is_simple_loop = true;
                        root->p_info_table->p_step_instr = p_instr;
                        return;
                    }
                }
            }
            free(root->p_info_table);
            root->p_info_table = NULL;
            return;
        }
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.p_des == root->p_info_table->p_cond_instr->ir_binary.p_src1->p_vreg) {
                if (p_instr->ir_binary.p_src1->kind == reg) {
                    if (p_instr->ir_binary.p_src1->p_vreg == root->p_info_table->step_var) {
                        if (check_operand(p_instr->ir_binary.p_src2)) {
                            root->is_simple_loop = true;
                            root->p_info_table->p_step_instr = p_instr;
                            return;
                        }
                    }
                }
                else if (p_instr->ir_binary.p_src2->kind == reg) {
                    if (p_instr->ir_binary.p_src2->p_vreg == root->p_info_table->step_var) {
                        if (check_operand(p_instr->ir_binary.p_src1)) {
                            root->is_simple_loop = true;
                            root->p_info_table->p_step_instr = p_instr;
                            return;
                        }
                    }
                }
                free(root->p_info_table);
                root->p_info_table = NULL;
                return;
            }
        }
    }
}

void basic_var_analysis(p_nestedtree_node root) {
    if (!root->head || !root->is_simple_loop) return;

    p_list_head p_node;
    p_ir_basic_block p_exit_block;
    if (!list_head_alone(&root->tail_list)) {
        p_list_head p_node;
        list_for_each(p_node, &root->tail_list) {
            p_exit_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            if (p_exit_block->p_branch->p_target_1->p_block == root->head || p_exit_block->p_branch->p_target_2->p_block == root->head) break;
        }
    }
    else
        p_exit_block = root->head;
    p_ir_basic_block_branch_target p_back_target;
    assert(p_exit_block->p_branch->p_target_1);
    if (p_exit_block->p_branch->p_target_1->p_block == root->head) {
        p_back_target = p_exit_block->p_branch->p_target_1;
    }
    else {
        assert(p_exit_block->p_branch->p_target_2);
        p_back_target = p_exit_block->p_branch->p_target_2;
    }
    p_ir_basic_block_branch_target p_entry_target;
    list_for_each(p_node, &root->head->prev_branch_target_list) {
        p_entry_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        if (!search(root->rbtree->root, (uint64_t) p_entry_target->p_source_block)) break;
    }
    // printf("entry %ld\n", p_entry_target->p_source_block->block_id);
    // printf("head %ld\n", root->head->block_id);
    // printf("barck %ld\n", p_back_target->p_source_block->block_id);
    for (p_list_head p_node1 = (&p_entry_target->block_param)->p_next,
                     p_node2 = (&root->head->basic_block_phis)->p_next,
                     p_node3 = (&p_back_target->block_param)->p_next;

         p_node2 != (&root->head->basic_block_phis);

         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next, p_node3 = p_node3->p_next) {
        p_ir_vreg p_vreg1 = list_entry(p_node1, ir_bb_param, node)->p_bb_param->p_vreg;
        p_ir_vreg p_vreg2 = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_vreg3 = list_entry(p_node3, ir_bb_param, node)->p_bb_param->p_vreg;
        // ir_vreg_print(p_vreg1);
        // putchar(10);
        // ir_vreg_print(p_vreg2);
        // putchar(10);
        // ir_vreg_print(p_vreg3);
        // putchar(10);
        p_list_head p_instr_node;
        bool flag = false;
        list_for_each(p_instr_node, &root->head->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.p_des == p_vreg3) {
                if (p_instr->ir_binary.p_src1->kind == reg) {
                    if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src2)) {
                        p_ind_var_list_node p_ind_var = malloc(sizeof(*p_ind_var));
                        *p_ind_var = (ind_var_list_node) {
                            .basic_ind_var = p_vreg2,
                            .start_var = p_vreg1,
                            .p_step_instr = p_instr,
                            .ind_var_table = list_head_init(&p_ind_var->ind_var_table),
                            .node = list_head_init(&p_ind_var->node),
                        };
                        list_add_next(&p_ind_var->node, &root->p_info_table->ind_var_list);
                    }
                }
                else if (p_instr->ir_binary.p_src2->kind == reg) {
                    if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src1)) {
                        p_ind_var_list_node p_ind_var = malloc(sizeof(*p_ind_var));
                        *p_ind_var = (ind_var_list_node) {
                            .basic_ind_var = p_vreg2,
                            .start_var = p_vreg1,
                            .p_step_instr = p_instr,
                            .ind_var_table = list_head_init(&p_ind_var->ind_var_table),
                            .node = list_head_init(&p_ind_var->node),
                        };
                        list_add_next(&p_ind_var->node, &root->p_info_table->ind_var_list);
                    }
                }
                flag = true;
                break;
            }
        }
        if (flag) continue;

        list_for_each(p_node, &root->tail_list) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_list_head p_instr_node;
            list_for_each(p_instr_node, &p_basic_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
                if (p_instr->irkind != ir_binary) continue;
                if (p_instr->ir_binary.p_des == p_vreg3) {
                    if (p_instr->ir_binary.p_src1->kind == reg) {
                        if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src2)) {
                            p_ind_var_list_node p_ind_var = malloc(sizeof(*p_ind_var));
                            *p_ind_var = (ind_var_list_node) {
                                .basic_ind_var = p_vreg2,
                                .start_var = p_vreg1,
                                .p_step_instr = p_instr,
                                .ind_var_table = list_head_init(&p_ind_var->ind_var_table),
                                .node = list_head_init(&p_ind_var->node),
                            };
                            list_add_next(&p_ind_var->node, &root->p_info_table->ind_var_list);
                        }
                    }
                    else if (p_instr->ir_binary.p_src2->kind == reg) {
                        if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src1)) {
                            p_ind_var_list_node p_ind_var = malloc(sizeof(*p_ind_var));
                            *p_ind_var = (ind_var_list_node) {
                                .basic_ind_var = p_vreg2,
                                .start_var = p_vreg1,
                                .p_step_instr = p_instr,
                                .ind_var_table = list_head_init(&p_ind_var->ind_var_table),
                                .node = list_head_init(&p_ind_var->node),
                            };
                            list_add_next(&p_ind_var->node, &root->p_info_table->ind_var_list);
                        }
                    }
                    break;
                }
            }
        }
    }
    return;
    list_for_each(p_node, &root->p_info_table->ind_var_list) {
        p_ind_var_list_node p_ind_var = list_entry(p_node, ind_var_list_node, node);
        putchar(10);
        ir_vreg_print(p_ind_var->basic_ind_var);
        putchar(10);
        ir_instr_print(p_ind_var->p_step_instr);
    }
}