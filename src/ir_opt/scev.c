#include <ir_opt/scev.h>

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

void program_var_analysis(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        printf("\n%s\n", p_func->name);
        loop_var_analysis(p_func->p_nestedtree_root);
    }
}

bool check_operand(p_ir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        if (p_operand->p_type->ref_level > 0) {
            return 0;
        }
        return 1;
    case reg:
        if (p_operand->p_vreg->is_loop_inv)
            return 1;
        return 0;
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
    basic_var_analysis(root);
    induction_var_analysis(root);

    secv_drop(root);
}

void invariant_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    if (!root->head) return;
    list_for_each(p_node, &root->head->basic_block_phis) {
        list_entry(p_node, ir_bb_phi, node)->p_bb_phi->is_loop_inv = false;
    }
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_binary:
            if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                p_instr->ir_binary.p_des->is_loop_inv = false;
            break;
        case ir_unary:
            if (!check_operand(p_instr->ir_unary.p_src))
                p_instr->ir_unary.p_des->is_loop_inv = false;
            break;
        case ir_load:
            p_instr->ir_load.p_des->is_loop_inv = false;
            break;
        case ir_gep:
            p_instr->ir_gep.p_des->is_loop_inv = false;
            break;
        case ir_call:
            if (p_instr->ir_call.p_func->ret_type != type_void)
                p_instr->ir_call.p_des->is_loop_inv = false;
            break;
        default:
            break;
        }
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_basic_block->basic_block_phis) {
            list_entry(p_list_node, ir_bb_phi, node)->p_bb_phi->is_loop_inv = false;
        }
        list_for_each(p_list_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_binary:
                if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                    p_instr->ir_binary.p_des->is_loop_inv = false;
                break;
            case ir_unary:
                if (!check_operand(p_instr->ir_unary.p_src))
                    p_instr->ir_unary.p_des->is_loop_inv = false;
                break;
            case ir_load:
                p_instr->ir_load.p_des->is_loop_inv = false;
                break;
            case ir_gep:
                p_instr->ir_gep.p_des->is_loop_inv = false;
                break;
            case ir_call:
                if (p_instr->ir_call.p_func->ret_type != type_void)
                    p_instr->ir_call.p_des->is_loop_inv = false;
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
            if (p_instr->ir_binary.p_des->is_loop_inv)
                printf("%ld\n", p_instr->ir_binary.p_des->id);
            break;
        case ir_unary:
            if (p_instr->ir_unary.p_des->is_loop_inv)
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
                    if (p_instr->ir_binary.p_des->is_loop_inv)
                        printf("%ld\n", p_instr->ir_binary.p_des->id);
                    break;
                case ir_unary:
                    if (p_instr->ir_unary.p_des->is_loop_inv)
                        printf("%ld\n", p_instr->ir_unary.p_des->id);
                default:
                    break;
                }
            }
        }
    }
}

void basic_var_analysis(p_nestedtree_node root) {
    if (!root->head) return;
    p_list_head p_node;
    p_ir_basic_block p_exit_block;
    if (!list_head_alone(&root->tail_list)) {
        p_list_head p_node;
        list_for_each(p_node, &root->tail_list) {
            p_exit_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            if (p_exit_block->p_branch->p_target_1->p_block == root->head || (p_exit_block->p_branch->p_target_2 && p_exit_block->p_branch->p_target_2->p_block == root->head)) break;
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
        p_vreg2->scev_kind = scev_basic;
        list_for_each(p_instr_node, &root->head->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.p_des == p_vreg3) {
                if (p_instr->ir_binary.p_src1->kind == reg) {
                    if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2) {
                        p_basic_var_info p_var = malloc(sizeof(*p_var));
                        *p_var = (basic_var_info) {
                            .var_start = p_vreg1,
                            .basic_var = p_vreg2,
                            .p_step_instr = p_instr,
                            .node = list_head_init(&p_var->node),
                        };
                        list_add_next(&p_var->node, &root->p_var_table);
                        p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                        *p_vreg2->p_scevexp = (scevexp) {
                            .is_scev1 = false,
                            .is_scev2 = false,
                            .p_var_info = p_var,
                            .p_operand2 = NULL,

                        };
                    }
                }
                else if (p_instr->ir_binary.p_src2->kind == reg) {
                    if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2) {
                        p_basic_var_info p_var = malloc(sizeof(*p_var));
                        *p_var = (basic_var_info) {
                            .var_start = p_vreg1,
                            .basic_var = p_vreg2,
                            .p_step_instr = p_instr,
                            .node = list_head_init(&p_var->node),
                        };
                        list_add_next(&p_var->node, &root->p_var_table);
                        p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                        *p_vreg2->p_scevexp = (scevexp) {
                            .is_scev1 = false,
                            .is_scev2 = false,
                            .p_var_info = p_var,
                            .p_operand2 = NULL,

                        };
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
                        if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2) {
                            flag = true;
                            p_basic_var_info p_var = malloc(sizeof(*p_var));
                            *p_var = (basic_var_info) {
                                .var_start = p_vreg1,
                                .basic_var = p_vreg2,
                                .p_step_instr = p_instr,
                                .node = list_head_init(&p_var->node),
                            };
                            list_add_next(&p_var->node, &root->p_var_table);
                            p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                            *p_vreg2->p_scevexp = (scevexp) {
                                .is_scev1 = false,
                                .is_scev2 = false,
                                .p_var_info = p_var,
                                .p_operand2 = NULL,
                            };
                        }
                    }
                    else if (p_instr->ir_binary.p_src2->kind == reg) {
                        if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2) {
                            flag = true;
                            p_basic_var_info p_var = malloc(sizeof(*p_var));
                            *p_var = (basic_var_info) {
                                .var_start = p_vreg1,
                                .basic_var = p_vreg2,
                                .p_step_instr = p_instr,
                                .node = list_head_init(&p_var->node),
                            };
                            list_add_next(&p_var->node, &root->p_var_table);
                            p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                            *p_vreg2->p_scevexp = (scevexp) {
                                .is_scev1 = false,
                                .is_scev2 = false,
                                .p_var_info = p_var,
                                .p_operand2 = NULL,
                            };
                        }
                    }
                    break;
                }
            }
        }
    }
    // return;
    printf("basic var %ld\n", root->head->block_id);
    list_for_each(p_node, &root->p_var_table) {
        p_basic_var_info p_var = list_entry(p_node, basic_var_info, node);
        putchar(10);
        ir_vreg_print(p_var->basic_var);
        putchar(10);
        ir_instr_print(p_var->p_step_instr);
    }
    putchar(10);
}

void induction_var_analysis(p_nestedtree_node root) {
    if (!root->head) return;
    p_list_head p_node;
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_unary:
            p_instr->ir_unary.p_des->scev_kind = scev_unknown;
            break;
        case ir_gep:
            p_instr->ir_gep.p_des->scev_kind = scev_unknown;
            break;
        case ir_store:
            break;
        case ir_load:
            p_instr->ir_load.p_des->scev_kind = scev_unknown;
            break;
        case ir_call:
            if (p_instr->ir_call.p_func->ret_type != type_void)
                p_instr->ir_call.p_des->scev_kind = scev_unknown;
            break;
        default:
            switch (p_instr->ir_binary.op) {
            case ir_add_op:
                p_instr->ir_binary.p_des->scev_kind = scev_add;
                break;
            case ir_sub_op:
                p_instr->ir_binary.p_des->scev_kind = scev_sub;
                break;
            case ir_mul_op:
                p_instr->ir_binary.p_des->scev_kind = scev_mul;
                break;
            case ir_div_op:
                p_instr->ir_binary.p_des->scev_kind = scev_div;
                break;
            case ir_mod_op:
                p_instr->ir_binary.p_des->scev_kind = scev_mod;
                break;
            default:
                p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                break;
            }
            if (p_instr->ir_binary.p_des->scev_kind == scev_unknown) break;
            if (p_instr->ir_binary.p_src1->kind == imme) {
                assert(p_instr->ir_binary.p_src2->kind == reg);
                if (p_instr->ir_binary.p_src2->p_vreg->scev_kind == scev_unknown) {
                    p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                    break;
                }
                p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                    .is_scev1 = false,
                    .is_scev2 = true,
                    .p_operand1 = p_instr->ir_binary.p_src1,
                    .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                };
            }
            else if (p_instr->ir_binary.p_src2->kind == imme) {
                assert(p_instr->ir_binary.p_src1->kind == reg);
                if (p_instr->ir_binary.p_src1->p_vreg->scev_kind == scev_unknown) {
                    p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                    break;
                }
                p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                    .is_scev1 = true,
                    .is_scev2 = false,
                    .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                    .p_operand2 = p_instr->ir_binary.p_src2,
                };
            }
            else {
                if (p_instr->ir_binary.p_src1->p_vreg->scev_kind == scev_unknown || p_instr->ir_binary.p_src2->p_vreg->scev_kind == scev_unknown) {
                    if (check_operand(p_instr->ir_binary.p_src1)) {
                        p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                        *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                            .is_scev1 = false,
                            .is_scev2 = true,
                            .p_operand1 = p_instr->ir_binary.p_src1,
                            .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                        };
                    }
                    else if (check_operand(p_instr->ir_binary.p_src2)) {
                        p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                        *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                            .is_scev1 = true,
                            .is_scev2 = false,
                            .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                            .p_operand2 = p_instr->ir_binary.p_src2,
                        };
                    }
                    else
                        p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                    break;
                }
                else {
                    p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                    *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                        .is_scev1 = true,
                        .is_scev2 = true,
                        .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                        .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                    };
                }
            }
        }
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_unary:
                p_instr->ir_unary.p_des->scev_kind = scev_unknown;
                break;
            case ir_gep:
                p_instr->ir_gep.p_des->scev_kind = scev_unknown;
                break;
            case ir_store:
                break;
            case ir_load:
                p_instr->ir_load.p_des->scev_kind = scev_unknown;
                break;
            case ir_call:
                if (p_instr->ir_call.p_func->ret_type != type_void)
                    p_instr->ir_call.p_des->scev_kind = scev_unknown;
                break;
            default:
                switch (p_instr->ir_binary.op) {
                case ir_add_op:
                    p_instr->ir_binary.p_des->scev_kind = scev_add;
                    break;
                case ir_sub_op:
                    p_instr->ir_binary.p_des->scev_kind = scev_sub;
                    break;
                case ir_mul_op:
                    p_instr->ir_binary.p_des->scev_kind = scev_mul;
                    break;
                case ir_div_op:
                    p_instr->ir_binary.p_des->scev_kind = scev_div;
                    break;
                case ir_mod_op:
                    p_instr->ir_binary.p_des->scev_kind = scev_mod;
                    break;
                default:
                    p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                    break;
                }
                if (p_instr->ir_binary.p_des->scev_kind == scev_unknown) break;
                if (p_instr->ir_binary.p_src1->kind == imme) {
                    assert(p_instr->ir_binary.p_src2->kind == reg);
                    if (p_instr->ir_binary.p_src2->p_vreg->scev_kind == scev_unknown) {
                        p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                        break;
                    }
                    p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                    *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                        .is_scev1 = false,
                        .is_scev2 = true,
                        .p_operand1 = p_instr->ir_binary.p_src1,
                        .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                    };
                }
                else if (p_instr->ir_binary.p_src2->kind == imme) {
                    assert(p_instr->ir_binary.p_src1->kind == reg);
                    if (p_instr->ir_binary.p_src1->p_vreg->scev_kind == scev_unknown) {
                        p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                        break;
                    }
                    p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                    *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                        .is_scev1 = true,
                        .is_scev2 = false,
                        .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                        .p_operand2 = p_instr->ir_binary.p_src2,
                    };
                }
                else {
                    if (p_instr->ir_binary.p_src1->p_vreg->scev_kind == scev_unknown || p_instr->ir_binary.p_src2->p_vreg->scev_kind == scev_unknown) {
                        if (check_operand(p_instr->ir_binary.p_src1)) {
                            p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                            *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                                .is_scev1 = false,
                                .is_scev2 = true,
                                .p_operand1 = p_instr->ir_binary.p_src1,
                                .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                            };
                        }
                        else if (check_operand(p_instr->ir_binary.p_src2)) {
                            p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                            *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                                .is_scev1 = true,
                                .is_scev2 = false,
                                .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                                .p_operand2 = p_instr->ir_binary.p_src2,
                            };
                        }
                        else
                            p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                        break;
                    }
                    else {
                        p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                        *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                            .is_scev1 = true,
                            .is_scev2 = true,
                            .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                            .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                        };
                    }
                }
            }
        }
    }
    // return;
    printf("secv\nhead %ld\n", root->head->block_id);
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        printf("%ld   ", p_instr->instr_id);
        if (p_instr->irkind != ir_binary) {
            printf("secv unkonwn\n");
            continue;
        }
        if (p_instr->ir_binary.p_des->scev_kind == scev_unknown) {
            printf("secv unkonwn\n");
            continue;
        }
        ir_vreg_print(p_instr->ir_binary.p_des);
        putchar(' ');
        ir_operand_print(p_instr->ir_binary.p_src1);
        switch (p_instr->ir_binary.p_des->scev_kind) {
        case scev_add:
            printf(" add ");
            break;
        case scev_sub:
            printf(" sub ");
            break;
        case scev_mul:
            printf(" mul ");
            break;
        case scev_div:
            printf(" div ");
            break;
        case scev_mod:
            printf(" mod ");
            break;
        default:
            break;
        }
        ir_operand_print(p_instr->ir_binary.p_src2);
        putchar(10);
    }
    putchar(10);
    list_for_each(p_node, &root->tail_list) {
        p_list_head p_instr_node;
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            printf("%ld   ", p_instr->instr_id);
            if (p_instr->irkind != ir_binary) {
                printf("secv unkonwn\n");
                continue;
            }
            if (p_instr->ir_binary.p_des->scev_kind == scev_unknown) {
                printf("secv unkonwn\n");
                continue;
            }
            ir_vreg_print(p_instr->ir_binary.p_des);
            putchar(' ');
            ir_operand_print(p_instr->ir_binary.p_src1);
            switch (p_instr->ir_binary.p_des->scev_kind) {
            case scev_add:
                printf(" add ");
                break;
            case scev_sub:
                printf(" sub ");
                break;
            case scev_mul:
                printf(" mul ");
                break;
            case scev_div:
                printf(" div ");
                break;
            case scev_mod:
                printf(" mod ");
                break;
            default:
                break;
            }
            ir_operand_print(p_instr->ir_binary.p_src2);
            putchar(10);
        }
    }
}

void secv_drop(p_nestedtree_node root) {
    if (!root->head) return;
    p_list_head p_node;
    list_for_each(p_node, &root->head->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        if (p_vreg->scev_kind == scev_unknown) continue;
        free(p_vreg->p_scevexp);
        p_vreg->p_scevexp = NULL;
    }
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        if (p_instr->irkind != ir_binary || p_instr->ir_binary.p_des->scev_kind == scev_unknown) continue;
        free(p_instr->ir_binary.p_des->p_scevexp);
        p_instr->ir_binary.p_des->p_scevexp = NULL;
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->basic_block_phis) {
            p_ir_vreg p_vreg = list_entry(p_instr_node, ir_bb_phi, node)->p_bb_phi;
            if (p_vreg->scev_kind == scev_unknown) continue;
            free(p_vreg->p_scevexp);
            p_vreg->p_scevexp = NULL;
        }
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind != ir_binary || p_instr->ir_binary.p_des->scev_kind == scev_unknown) continue;
            free(p_instr->ir_binary.p_des->p_scevexp);
            p_instr->ir_binary.p_des->p_scevexp = NULL;
        }
    }
}