
#include <ir_opt/scev.h>

#include <util.h>

#include <ir/basic_block.h>
#include <stdio.h>

#include <ir/bb_param.h>
#include <ir/instr.h>
#include <ir/operand.h>
#include <ir/param.h>
#include <ir/vreg.h>
#include <ir_gen.h>
#include <symbol/var.h>
#include <symbol_gen/func.h>

#include <ir_print.h>

#include <symbol/type.h>

void program_var_analysis(p_program p_program, bool if_opt) {
    printf("\n------ var analysis begin -------\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        printf("\n%s\n", p_func->name);
        p_list_head p_vreg_node;
        list_for_each(p_vreg_node, &p_func->param_reg_list) {
            list_entry(p_vreg_node, ir_vreg, node)->is_loop_inv = true;
        }
        list_for_each(p_vreg_node, &p_func->vreg_list) {
            list_entry(p_vreg_node, ir_vreg, node)->is_loop_inv = true;
        }

        loop_var_analysis(p_func->p_nestedtree_root, if_opt);
        symbol_func_set_block_id(p_func);
        p_list_head p_list_head;
        list_for_each(p_list_head, &p_func->vreg_list) {
            p_ir_vreg p_vreg = list_entry(p_list_head, ir_vreg, node);
            p_vreg->scev_kind = scev_unknown;
            if (p_vreg->p_scevexp == NULL) continue;
            free(p_vreg->p_scevexp);
            p_vreg->p_scevexp = NULL;
        }
    }
    printf("\n------ var analysis end -------\n");
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

bool cmp_vreg(p_ir_operand p_operand, p_ir_vreg p_vreg) {
    if (p_operand->kind == reg && p_operand->p_vreg == p_vreg)
        return 1;
    return 0;
}

void dfs_loop_block(HashTable *hash, p_ir_basic_block p_basic_block) {
    hashinsert(hash, (uint64_t) p_basic_block, 1);
    if (p_basic_block->p_branch->kind == ir_br_branch)
        dfs_loop_block(hash, p_basic_block->p_branch->p_target_1->p_block);
}

void instr_insert(ir_binary_op op, p_ir_operand p1, p_ir_operand p2, p_ir_vreg p_des, p_ir_basic_block p_block) {
    p_ir_instr p_instr = ir_binary_instr_gen(op, p1, p2, p_des);
    ir_basic_block_addinstr_tail(p_block, p_instr);
    symbol_func_vreg_add(p_block->p_func, p_des);
}

void loop_var_analysis(p_nestedtree_node root, bool if_opt) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nestedtree_node p_list_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        loop_var_analysis(p_list_node, if_opt);
    }
    // invariant analysis
    if (!root->head) return;
    invariant_analysis(root);
    basic_var_analysis(root);
    induction_var_analysis(root);
    loop_info_analysis(root);
    // loop opt
    if (if_opt) {
        var_strength_reduction(root);
        if (root->p_loop_step) {
            accumulation_analysis(root);
        }
    }

    secv_drop(root);
    // info drop
}

void loop_info_analysis(p_nestedtree_node root) {
    printf("loop head %ld: loop info\n", root->head->block_id);
    p_ir_basic_block p_exit_block = NULL;
    p_list_head p_node;
    list_for_each(p_node, &root->p_loop_latch_block->prev_branch_target_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
        if (p_exit_block) return;
        p_exit_block = p_block;
    }
    list_for_each(p_node, &p_exit_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        if (p_instr->irkind != ir_binary) continue;
        if (p_instr->ir_binary.p_des->if_cond) {
            if (p_instr->ir_binary.op != ir_l_op && p_instr->ir_binary.op != ir_leq_op && p_instr->ir_binary.op != ir_g_op && p_instr->ir_binary.op != ir_geq_op) return;
            p_list_head p_list_node;
            list_for_each(p_list_node, &root->p_var_table) {
                p_basic_var_info p_info = list_entry(p_list_node, basic_var_info, node);
                if (p_instr->ir_binary.p_src1->kind == reg && p_instr->ir_binary.p_src1->p_vreg == p_info->p_step_instr->ir_binary.p_des && check_operand(p_instr->ir_binary.p_src2)) {
                    root->p_loop_step = malloc(sizeof(*root->p_loop_step));
                    *root->p_loop_step = (loop_step_info) {
                        .p_cond_instr = p_instr,
                        .p_basic_var = p_info,
                        .is_xeq = false,
                    };
                    if (p_instr->ir_binary.op == ir_leq_op || p_instr->ir_binary.op == ir_geq_op) root->p_loop_step->is_xeq = true;
                    return;
                }
                if (p_instr->ir_binary.p_src2->kind == reg && p_instr->ir_binary.p_src2->p_vreg == p_info->p_step_instr->ir_binary.p_des && check_operand(p_instr->ir_binary.p_src1)) {
                    p_ir_operand p1 = ir_operand_copy(p_instr->ir_binary.p_src2);
                    p_ir_operand p2 = ir_operand_copy(p_instr->ir_binary.p_src1);
                    switch (p_instr->ir_binary.op) {
                    case ir_leq_op:
                        p_instr->ir_binary.op = ir_geq_op;
                        break;
                    case ir_geq_op:
                        p_instr->ir_binary.op = ir_leq_op;
                        break;
                    case ir_l_op:
                        p_instr->ir_binary.op = ir_g_op;
                        break;
                    case ir_g_op:
                        p_instr->ir_binary.op = ir_l_op;
                        break;
                    default:
                        assert(1);
                        break;
                    }
                    ir_instr_reset_binary(p_instr, p_instr->ir_binary.op, p1, p2, p_instr->ir_binary.p_des);
                    root->p_loop_step = malloc(sizeof(*root->p_loop_step));
                    *root->p_loop_step = (loop_step_info) {
                        .p_cond_instr = p_instr,
                        .p_basic_var = p_info,
                        .is_xeq = false,
                    };
                    if (p_instr->ir_binary.op == ir_leq_op || p_instr->ir_binary.op == ir_geq_op) root->p_loop_step->is_xeq = true;
                    return;
                }
            }
        }
    }
}

void scev_add_instr(p_ir_vreg p_vreg, p_sum_info p_info, p_ir_basic_block p_block, bool is_sub) {
    if (p_vreg->scev_kind == scev_basic) {
        if (p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->p_vreg == p_vreg) {
            p_ir_operand p1 = ir_operand_vreg_gen(p_info->add_const);
            p_ir_operand p2 = ir_operand_vreg_gen(p_vreg->p_scevexp->p_var_info->var_start);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            instr_insert(is_sub, p1, p2, p_info->add_const, p_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->add_var);
            if (p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src2->kind == imme)
                if (p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->p_type->basic == type_i32)
                    p2 = ir_operand_int_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src2->i32const);
                else
                    p2 = ir_operand_int_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src2->f32const);
            else
                p2 = ir_operand_vreg_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src2->p_vreg);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.op, p1, p2, p_info->add_var, p_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        }
        else {
            p_ir_operand p1 = ir_operand_vreg_gen(p_info->add_const);
            p_ir_operand p2 = ir_operand_vreg_gen(p_vreg->p_scevexp->p_var_info->var_start);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            instr_insert(is_sub, p1, p2, p_info->add_const, p_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->add_var);
            if (p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->kind == imme) {
                if (p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->p_type->basic == type_i32)
                    p2 = ir_operand_int_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->i32const);
                else
                    p2 = ir_operand_int_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->f32const);
            }

            else
                p2 = ir_operand_vreg_gen(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.p_src1->p_vreg);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->p_scevexp->p_var_info->p_step_instr->ir_binary.op, p1, p2, p_info->add_var, p_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        }
        return;
    }
    if (p_vreg->p_scevexp->is_scev1 && !p_vreg->p_scevexp->is_scev2) {
        scev_add_instr(p_vreg->p_scevexp->p_scev1->p_des, p_info, p_block, is_sub);
        p_ir_operand p1, p2;
        if (p_vreg->scev_kind == scev_mul) {
            p1 = ir_operand_vreg_gen(p_info->add_var);
            p2 = ir_operand_copy(p_vreg->p_scevexp->p_operand2);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->scev_kind, p1, p2, p_info->add_var, p_block);
        }
        p1 = ir_operand_vreg_gen(p_info->add_const);
        p2 = ir_operand_copy(p_vreg->p_scevexp->p_operand2);
        p_info->add_const = ir_vreg_copy(p1->p_vreg);
        instr_insert(p_vreg->scev_kind, p1, p2, p_info->add_const, p_block);
    }
    else if (!p_vreg->p_scevexp->is_scev1 && p_vreg->p_scevexp->is_scev2) {
        if (p_vreg->scev_kind == scev_sub) is_sub = !is_sub;
        scev_add_instr(p_vreg->p_scevexp->p_scev2->p_des, p_info, p_block, is_sub);
        p_ir_operand p1, p2;
        if (p_vreg->scev_kind == scev_mul) {
            p1 = ir_operand_vreg_gen(p_info->add_var);
            p2 = ir_operand_copy(p_vreg->p_scevexp->p_operand1);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->scev_kind, p1, p2, p_info->add_var, p_block);
        }
        p1 = ir_operand_vreg_gen(p_info->add_const);
        p2 = ir_operand_copy(p_vreg->p_scevexp->p_operand1);
        p_info->add_const = ir_vreg_copy(p1->p_vreg);
        instr_insert(ir_add_op, p1, p2, p_info->add_const, p_block);
    }
    else {
        assert(p_vreg->scev_kind == scev_add || p_vreg->scev_kind == scev_sub);

        p_sum_info p_new_info = malloc(sizeof(*p_new_info));
        *p_new_info = (sum_info) {
            .add_const = NULL,
            .add_var = NULL,
            .count = NULL,
            .mul_count = NULL,
            .instr_list = list_head_init(&p_new_info->instr_list),
        };
        p_ir_operand p1, p2;
        if (p_info->add_const->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_const = ir_vreg_copy(p_info->add_const);
        p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
        symbol_func_vreg_add(p_block->p_func, p_new_info->add_const);
        ir_basic_block_addinstr_tail(p_block, p_add_instr);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        if (p_info->add_var->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_var = ir_vreg_copy(p_info->add_var);
        p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
        symbol_func_vreg_add(p_block->p_func, p_new_info->add_var);
        ir_basic_block_addinstr_tail(p_block, p_add_instr);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        scev_add_instr(p_vreg->p_scevexp->p_scev1->p_des, p_new_info, p_block, is_sub);
        if (p_new_info->add_const) {
            p1 = ir_operand_vreg_gen(p_info->add_const);
            p2 = ir_operand_vreg_gen(p_new_info->add_const);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_add_op, p1, p2, p_info->add_const, p_block);
        }
        if (p_new_info->add_var) {
            p1 = ir_operand_vreg_gen(p_info->add_var);
            p2 = ir_operand_vreg_gen(p_new_info->add_var);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_add_op, p1, p2, p_info->add_var, p_block);
        }
        free(p_new_info);

        p_new_info = malloc(sizeof(*p_new_info));
        *p_new_info = (sum_info) {
            .add_const = NULL,
            .add_var = NULL,
            .count = NULL,
            .mul_count = NULL,
            .instr_list = list_head_init(&p_new_info->instr_list),
        };
        if (p_info->add_const->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_const = ir_vreg_copy(p_info->add_const);
        p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
        symbol_func_vreg_add(p_block->p_func, p_new_info->add_const);
        ir_basic_block_addinstr_tail(p_block, p_add_instr);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        if (p_info->add_var->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_var = ir_vreg_copy(p_info->add_var);
        p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
        symbol_func_vreg_add(p_block->p_func, p_new_info->add_var);
        ir_basic_block_addinstr_tail(p_block, p_add_instr);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        if (p_vreg->scev_kind == scev_sub) is_sub = !is_sub;
        scev_add_instr(p_vreg->p_scevexp->p_scev2->p_des, p_new_info, p_block, is_sub);
        if (p_new_info->add_const) {
            p1 = ir_operand_vreg_gen(p_info->add_const);
            p2 = ir_operand_vreg_gen(p_new_info->add_const);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->scev_kind, p1, p2, p_info->add_const, p_block);
        }
        if (p_new_info->add_var) {
            p1 = ir_operand_vreg_gen(p_info->add_var);
            p2 = ir_operand_vreg_gen(p_new_info->add_var);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(p_vreg->scev_kind, p1, p2, p_info->add_var, p_block);
        }
        free(p_new_info);
        // scev_add_instr(p_vreg->p_scevexp->p_scev1->p_des, p_info, p_block);
        // scev_add_instr(p_vreg->p_scevexp->p_scev2->p_des, p_info, p_block);
    }
}

void accumulation_analysis(p_nestedtree_node root) {
    if (root->p_loop_step == NULL) return;
    printf("loop head %ld: accumulation\n", root->head->block_id);
    int exit_num = 0;
    p_list_head p_node;
    p_ir_basic_block p_exit_block;
    if (root->head->is_loop_exit) ++exit_num;
    list_for_each(p_node, &root->tail_list) {
        if (list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block->is_loop_exit) ++exit_num;
        if (exit_num > 1) return;
    }

    int cnt = 0;
    list_for_each(p_node, &root->loop_exit_block) {
        if (++cnt > 1) return;
    }
    assert(cnt == 1);

    p_ir_basic_block_branch_target p_back_target = NULL;
    list_for_each(p_node, &root->p_loop_latch_block->prev_branch_target_list) {
        if (p_back_target) return;
        p_back_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        p_exit_block = p_back_target->p_source_block;
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
        p_ir_vreg p_vreg3 = list_entry(p_node1, ir_bb_param, node)->p_bb_param->p_vreg;
        p_ir_vreg p_vreg1 = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_vreg2 = list_entry(p_node3, ir_bb_param, node)->p_bb_param->p_vreg;
        if (p_vreg1 == root->p_loop_step->p_basic_var->basic_var && p_vreg2->def_type != instr_def) continue;
        bool flag = false;
        list_head p_sum_list = list_head_init(&p_sum_list);
        p_list_head p_instr_node;
        p_list_head p_param_node;
        HashTable *hashtable = initHashTable(19777);
        HashTable *hashtable_instr = initHashTable(19777);
        // printf("vreg %ld\n", p_vreg1->id);
        if (p_vreg1->p_type->basic != type_i32) flag = true;
        hashinsert(hashtable, (uint64_t) p_vreg1, 1);
        int cnt = 0, is_param = 0;
        p_list_head p_operand_node;
        list_for_each(p_operand_node, &p_vreg1->use_list) {
            p_ir_operand p_operand = list_entry(p_operand_node, ir_operand, use_node);

            if (p_operand->used_type == instr_ptr && search(root->rbtree->root, (uint64_t) p_operand->p_instr->p_basic_block))
                cnt = cnt + 1;

            else if (p_operand->used_type == bb_param_ptr)
                is_param = 1;
            else if (search(root->rbtree->root, (uint64_t) p_operand->p_basic_block))
                flag = true;
        }
        if (cnt + is_param != 1) flag = true;

        int max_depth = root->head->dom_depth;
        for (int i = root->head->dom_depth; i <= max_depth && !flag; ++i) {
            p_list_head p_block_node;
            list_for_each(p_block_node, &root->head->loop_node_list) {
                if (flag) break;
                p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block_list_node, node)->p_basic_block;
                max_depth = p_block->dom_depth > max_depth ? p_block->dom_depth : max_depth;
                if (p_block->p_nestree_node != root || p_block->dom_depth != i) continue;
                list_for_each(p_instr_node, &p_block->instr_list) {
                    p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
                    if (hashfind(hashtable_instr, (uint64_t) p_instr)) continue;
                    switch (p_instr->irkind) {
                    case ir_unary:
                        if (p_instr->ir_unary.p_src->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_unary.p_src->p_vreg))
                            flag = true;

                        break;
                    case ir_load:
                        if (p_instr->ir_load.p_addr->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_load.p_addr->p_vreg))
                            flag = true;

                        break;
                    case ir_gep:
                        if (p_instr->ir_gep.p_offset->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_gep.p_offset->p_vreg))
                            flag = true;

                        if (p_instr->ir_gep.p_addr->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_gep.p_addr->p_vreg))
                            flag = true;

                        break;
                    case ir_store:
                        if (p_instr->ir_store.p_src->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_store.p_src->p_vreg))
                            flag = true;

                        if (p_instr->ir_store.p_addr->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_store.p_addr->p_vreg))
                            flag = true;

                        break;
                    case ir_call:
                        list_for_each(p_param_node, &p_instr->ir_call.param_list) {
                            p_ir_param p_param = list_entry(p_param_node, ir_param, node);
                            if (p_param->is_in_mem || p_param->p_param->kind != reg) continue;
                            if (hashfind(hashtable, (uint64_t) p_param->p_param->p_vreg)) {
                                flag = true;

                                break;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                    if (p_instr->irkind != ir_binary) continue;
                    if (p_instr->ir_binary.op == ir_add_op || p_instr->ir_binary.op == ir_sub_op) {
                        if (cmp_vreg(p_instr->ir_binary.p_src1, p_vreg1)) {
                            if (p_instr->ir_binary.p_src1 == p_instr->ir_binary.p_src2) {
                                flag = true;

                                break;
                            }
                            if (p_instr->ir_binary.p_src2->kind == imme || (p_instr->ir_binary.p_src2->kind == reg && (p_instr->ir_binary.p_src2->p_vreg->is_loop_inv || p_instr->ir_binary.p_src2->p_vreg->scev_kind != scev_unknown))) {
                                p_sum_instr_node p_sum_instr = malloc(sizeof(*p_sum_instr));
                                --i;
                                *p_sum_instr = (sum_instr_node) {
                                    .p_instr = p_instr,
                                    .node = list_head_init(&p_sum_instr->node),
                                };
                                list_add_prev(&p_sum_instr->node, &p_sum_list);
                                p_vreg1 = p_instr->ir_binary.p_des;
                                // printf("vreg %ld\n", p_vreg1->id);
                                hashinsert(hashtable, (uint64_t) p_vreg1, 1);
                                hashinsert(hashtable_instr, (uint64_t) p_instr, 1);
                                int cnt = 0, is_param = 0;
                                p_list_head p_operand_node;
                                list_for_each(p_operand_node, &p_vreg1->use_list) {
                                    p_ir_operand p_operand = list_entry(p_operand_node, ir_operand, use_node);

                                    if (p_operand->used_type == instr_ptr && search(root->rbtree->root, (uint64_t) p_operand->p_instr->p_basic_block))
                                        cnt = cnt + 1;

                                    else if (p_operand->used_type == bb_param_ptr)
                                        is_param = 1;
                                    else if (search(root->rbtree->root, (uint64_t) p_operand->p_basic_block))
                                        flag = true;
                                }
                                if (cnt + is_param != 1) flag = true;

                                if (p_vreg1->p_type->basic != type_i32) flag = true;

                                continue;
                            }
                        }
                        else if (cmp_vreg(p_instr->ir_binary.p_src2, p_vreg1)) {
                            if (p_instr->ir_binary.p_src1 == p_instr->ir_binary.p_src2) {
                                flag = true;

                                break;
                            }
                            if (p_instr->ir_binary.p_src1->kind == imme || (p_instr->ir_binary.p_src1->kind == reg && (p_instr->ir_binary.p_src1->p_vreg->is_loop_inv || p_instr->ir_binary.p_src1->p_vreg->scev_kind != scev_unknown))) {
                                p_sum_instr_node p_sum_instr = malloc(sizeof(*p_sum_instr));
                                --i;
                                *p_sum_instr = (sum_instr_node) {
                                    .p_instr = p_instr,
                                    .node = list_head_init(&p_sum_instr->node),
                                };
                                list_add_prev(&p_sum_instr->node, &p_sum_list);
                                p_vreg1 = p_instr->ir_binary.p_des;
                                // printf("vreg %ld\n", p_vreg1->id);
                                hashinsert(hashtable, (uint64_t) p_vreg1, 1);
                                hashinsert(hashtable_instr, (uint64_t) p_instr, 1);
                                int cnt = 0, is_param = 0;
                                p_list_head p_operand_node;
                                list_for_each(p_operand_node, &p_vreg1->use_list) {
                                    p_ir_operand p_operand = list_entry(p_operand_node, ir_operand, use_node);

                                    if (p_operand->used_type == instr_ptr && search(root->rbtree->root, (uint64_t) p_operand->p_instr->p_basic_block))
                                        cnt = cnt + 1;

                                    else if (p_operand->used_type == bb_param_ptr)
                                        is_param = 1;
                                    else if (search(root->rbtree->root, (uint64_t) p_operand->p_basic_block))
                                        flag = true;
                                }
                                if (cnt + is_param != 1) flag = true;

                                if (p_vreg1->p_type->basic != type_i32) flag = true;

                                continue;
                            }
                        }
                        else {
                            if (p_instr->ir_binary.p_src1->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_binary.p_src1->p_vreg))
                                flag = true;

                            if (p_instr->ir_binary.p_src2->kind == reg && hashfind(hashtable, (uint64_t) p_instr->ir_binary.p_src2->p_vreg))
                                flag = true;
                        }
                    }
                }
            }
        }
        if (p_vreg1 == p_vreg2 && !flag && p_vreg1->def_type == instr_def) {

            printf("find reduction %ld\n", p_vreg1->id);
            ir_instr_print(p_vreg1->p_instr_def);
            // list_for_each(p_list_node, &p_sum_list) {
            //     p_sum_instr_node p_sum_instr = list_entry(p_list_node, sum_instr_node, node);
            //  ir_instr_print(p_sum_instr->p_instr);
            //}

            p_sum_info p_info = malloc(sizeof(*p_info));
            *p_info = (sum_info) {
                .add_const = NULL,
                .add_var = NULL,
                .count = NULL,
                .mul_count = NULL,
                .instr_list = list_head_init(&p_info->instr_list),
            };
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
            // if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_l_op || root->p_loop_step->p_cond_instr->ir_binary.op == ir_leq_op)
            instr_insert(ir_sub_op, p1, p2, p_count, p_exit_block);
            // else
            //    instr_insert(ir_sub_op, p2, p1, p_count, p_exit_block);
            p1 = ir_operand_vreg_gen(p_count);
            p2 = ir_operand_copy(root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2);
            p_count = ir_vreg_copy(p_count);
            instr_insert(ir_add_op, p1, p2, p_count, p_exit_block);

            if (!root->p_loop_step->is_xeq) {
                p1 = ir_operand_vreg_gen(p_count);
                if (p_count->p_type->basic == type_i32)
                    p2 = ir_operand_int_gen(1);
                else
                    p2 = ir_operand_float_gen(1.0);
                p_count = ir_vreg_copy(p_count);
                if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_l_op)
                    instr_insert(ir_sub_op, p1, p2, p_count, p_exit_block);
                else if (root->p_loop_step->p_cond_instr->ir_binary.op == ir_g_op)
                    instr_insert(ir_add_op, p1, p2, p_count, p_exit_block);
                // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            }

            p1 = ir_operand_vreg_gen(p_count);
            p2 = ir_operand_copy(root->p_loop_step->p_basic_var->p_step_instr->ir_binary.p_src2);
            p_info->count = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_div_op, p1, p2, p_info->count, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->count);
            if (p_info->count->p_type->basic == type_i32)
                p2 = ir_operand_int_gen(1);
            else
                p2 = ir_operand_float_gen(1.0);
            p_count = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_sub_op, p1, p2, p_count, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p2 = ir_operand_copy(p1);
            p1 = ir_operand_vreg_gen(p_count);
            p_info->mul_count = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_mul_op, p1, p2, p_info->mul_count, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->mul_count);
            if (p_info->mul_count->p_type->basic == type_i32)
                p2 = ir_operand_int_gen(2);
            else
                p2 = ir_operand_float_gen(2.0);
            p_info->mul_count = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_div_op, p1, p2, p_info->mul_count, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            if (p_info->add_const->p_type->basic == type_i32)
                p1 = ir_operand_int_gen(0);
            else
                p1 = ir_operand_float_gen(0.0);
            p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_info->add_const);
            symbol_func_vreg_add(p_exit_block->p_func, p_info->add_const);
            ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            if (p_info->add_var->p_type->basic == type_i32)
                p1 = ir_operand_int_gen(0);
            else
                p1 = ir_operand_float_gen(0.0);
            p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_info->add_var);
            symbol_func_vreg_add(p_exit_block->p_func, p_info->add_var);
            ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p_list_head p_list_node;
            p_list_node = (&p_sum_list)->p_next;
            p_ir_instr p_sum_instr = list_entry(p_list_node, sum_instr_node, node)->p_instr;
            if (p_sum_instr->ir_binary.p_src1->kind == reg && hashfind(hashtable, (uint64_t) p_sum_instr->ir_binary.p_src1->p_vreg)) {
                if (p_sum_instr->ir_binary.p_src2->kind == imme) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_copy(p_sum_instr->ir_binary.p_src2);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else if (p_sum_instr->ir_binary.p_src2->p_vreg->is_loop_inv) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_vreg_gen(p_sum_instr->ir_binary.p_src2->p_vreg);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else {
                    p_sum_info p_new_info = malloc(sizeof(*p_new_info));
                    *p_new_info = (sum_info) {
                        .add_const = NULL,
                        .add_var = NULL,
                        .count = NULL,
                        .mul_count = NULL,
                        .instr_list = list_head_init(&p_new_info->instr_list),
                    };
                    if (p_info->add_const->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_const = ir_vreg_copy(p_info->add_const);
                    p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_const);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    if (p_info->add_var->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_var = ir_vreg_copy(p_info->add_var);
                    p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_var);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    scev_add_instr(p_sum_instr->ir_binary.p_src2->p_vreg, p_new_info, p_exit_block, false);
                    if (p_new_info->add_const) {
                        p1 = ir_operand_vreg_gen(p_info->add_const);
                        p2 = ir_operand_vreg_gen(p_new_info->add_const);
                        p_info->add_const = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_const, p_exit_block);
                    }
                    if (p_new_info->add_var) {
                        p1 = ir_operand_vreg_gen(p_info->add_var);
                        p2 = ir_operand_vreg_gen(p_new_info->add_var);
                        p_info->add_var = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_var, p_exit_block);
                    }
                    free(p_new_info);
                }
            }
            else {
                if (p_sum_instr->ir_binary.p_src1->kind == imme) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_copy(p_sum_instr->ir_binary.p_src1);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else if (p_sum_instr->ir_binary.p_src1->p_vreg->is_loop_inv) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_vreg_gen(p_sum_instr->ir_binary.p_src1->p_vreg);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else {
                    p_sum_info p_new_info = malloc(sizeof(*p_new_info));
                    *p_new_info = (sum_info) {
                        .add_const = NULL,
                        .add_var = NULL,
                        .count = NULL,
                        .mul_count = NULL,
                        .instr_list = list_head_init(&p_new_info->instr_list),
                    };
                    p_ir_operand p1;
                    if (p_info->add_const->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_const = ir_vreg_copy(p_info->add_const);
                    p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_const);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    if (p_info->add_var->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_var = ir_vreg_copy(p_info->add_var);
                    p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_var);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    scev_add_instr(p_sum_instr->ir_binary.p_src1->p_vreg, p_new_info, p_exit_block, false);
                    if (p_new_info->add_const) {
                        p1 = ir_operand_vreg_gen(p_info->add_const);
                        p2 = ir_operand_vreg_gen(p_new_info->add_const);
                        p_info->add_const = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_const, p_exit_block);
                    }
                    if (p_new_info->add_var) {
                        p1 = ir_operand_vreg_gen(p_info->add_var);
                        p2 = ir_operand_vreg_gen(p_new_info->add_var);
                        p_info->add_var = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_var, p_exit_block);
                    }
                    free(p_new_info);
                }
            }
            for (p_list_node = p_list_node->p_next; p_list_node != (&p_sum_list); p_list_node = p_list_node->p_next) {
                p_sum_instr = list_entry(p_list_node, sum_instr_node, node)->p_instr;
                if (p_sum_instr->ir_binary.p_src2->kind == imme) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_copy(p_sum_instr->ir_binary.p_src2);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else if (p_sum_instr->ir_binary.p_src2->p_vreg->is_loop_inv) {
                    p1 = ir_operand_vreg_gen(p_info->add_const);
                    p2 = ir_operand_vreg_gen(p_sum_instr->ir_binary.p_src2->p_vreg);
                    p_info->add_const = ir_vreg_copy(p1->p_vreg);
                    instr_insert(p_sum_instr->ir_binary.op, p1, p2, p_info->add_const, p_exit_block);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                }
                else {
                    p_sum_info p_new_info = malloc(sizeof(*p_new_info));
                    *p_new_info = (sum_info) {
                        .add_const = NULL,
                        .add_var = NULL,
                        .count = NULL,
                        .mul_count = NULL,
                        .instr_list = list_head_init(&p_new_info->instr_list),
                    };
                    p_ir_operand p1;
                    if (p_info->add_const->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_const = ir_vreg_copy(p_info->add_const);
                    p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_const);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    if (p_info->add_var->p_type->basic == type_i32)
                        p1 = ir_operand_int_gen(0);
                    else
                        p1 = ir_operand_float_gen(0.0);
                    p_new_info->add_var = ir_vreg_copy(p_info->add_var);
                    p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
                    symbol_func_vreg_add(p_exit_block->p_func, p_new_info->add_var);
                    ir_basic_block_addinstr_tail(p_exit_block, p_add_instr);
                    // list_add_prev(&p_add_instr->node, &p_info->instr_list);
                    if (p_sum_instr->ir_binary.p_src2->p_vreg->scev_kind != scev_unknown)
                        scev_add_instr(p_sum_instr->ir_binary.p_src2->p_vreg, p_new_info, p_exit_block, false);
                    else
                        scev_add_instr(p_sum_instr->ir_binary.p_src1->p_vreg, p_new_info, p_exit_block, false);
                    if (p_new_info->add_const) {
                        p1 = ir_operand_vreg_gen(p_info->add_const);
                        p2 = ir_operand_vreg_gen(p_new_info->add_const);
                        p_info->add_const = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_const, p_exit_block);
                    }
                    if (p_new_info->add_var) {
                        p1 = ir_operand_vreg_gen(p_info->add_var);
                        p2 = ir_operand_vreg_gen(p_new_info->add_var);
                        p_info->add_var = ir_vreg_copy(p1->p_vreg);
                        instr_insert(ir_add_op, p1, p2, p_info->add_var, p_exit_block);
                    }
                    free(p_new_info);
                }
            }
            p1 = ir_operand_vreg_gen(p_info->add_const);
            p2 = ir_operand_vreg_gen(p_info->count);
            p_info->add_const = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_mul_op, p1, p2, p_info->add_const, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->add_var);
            p2 = ir_operand_vreg_gen(p_info->mul_count);
            p_info->add_var = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_mul_op, p1, p2, p_info->add_var, p_exit_block);
            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            p1 = ir_operand_vreg_gen(p_info->add_const);
            p2 = ir_operand_vreg_gen(p_info->add_var);
            p_count = ir_vreg_copy(p1->p_vreg);
            instr_insert(ir_add_op, p1, p2, p_count, p_exit_block);
            p1 = ir_operand_vreg_gen(p_vreg3);
            p2 = ir_operand_vreg_gen(p_count);
            assert(p_vreg2->def_type == instr_def);
            ir_instr_reset_binary(p_vreg2->p_instr_def, ir_add_op, p1, p2, p_vreg2);

            // list_add_prev(&p_add_instr->node, &p_info->instr_list);
            printf("\n");
            // list_for_each(p_list_node, &p_info->instr_list) {
            //     p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            //     ir_instr_print(p_instr);
            // }
            // list_add_next(&p_info->instr_list, &root->head->instr_list);
            free(p_info);
        }
        while (!list_head_alone(&p_sum_list)) {
            p_sum_instr_node p_sum_instr = list_entry(p_sum_list.p_next, sum_instr_node, node);
            list_del(&p_sum_instr->node);
            free(p_sum_instr);
        }
        destroyHashTable(hashtable);
        destroyHashTable(hashtable_instr);
    }
}

void invariant_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    int max_depth = root->head->dom_depth;
    for (int i = root->head->dom_depth; i <= max_depth; ++i) {
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            max_depth = p_basic_block->dom_depth > max_depth ? p_basic_block->dom_depth : max_depth;
            if (p_basic_block->p_nestree_node != root || p_basic_block->dom_depth != i) continue;
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
                    if (p_instr->ir_unary.op != ir_val_assign && p_instr->ir_unary.op != ir_minus_op) {
                        p_instr->ir_unary.p_des->is_loop_inv = false;
                        break;
                    }
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
    }

    // return;
    printf("head %ld: inv var\n", root->head->block_id);
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
    p_list_head p_node;
    p_ir_basic_block_branch_target p_back_target = NULL;
    list_for_each(p_node, &root->p_loop_latch_block->prev_branch_target_list) {
        if (p_back_target) return;
        p_back_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
    }
    p_ir_basic_block_branch_target p_entry_target;
    if (root->p_loop_pre_block->p_branch->p_target_1->p_block == root->head)
        p_entry_target = root->p_loop_pre_block->p_branch->p_target_1;
    else
        p_entry_target = root->p_loop_pre_block->p_branch->p_target_2;
    for (p_list_head p_node1 = (&p_entry_target->block_param)->p_next,
                     p_node2 = (&root->head->basic_block_phis)->p_next,
                     p_node3 = (&p_back_target->block_param)->p_next;

         p_node2 != (&root->head->basic_block_phis);

         p_node1 = p_node1->p_next, p_node2 = p_node2->p_next, p_node3 = p_node3->p_next) {
        p_ir_vreg p_vreg1 = list_entry(p_node1, ir_bb_param, node)->p_bb_param->p_vreg;
        p_ir_vreg p_vreg2 = list_entry(p_node2, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_vreg3 = list_entry(p_node3, ir_bb_param, node)->p_bb_param->p_vreg;
        p_list_head p_instr_node;
        bool flag = false;
        list_for_each(p_instr_node, &root->head->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind != ir_binary) continue;
            if (p_instr->ir_binary.op != ir_add_op && p_instr->ir_binary.op != ir_sub_op) continue;
            if (p_instr->ir_binary.p_des == p_vreg3) {
                if (p_instr->ir_binary.p_src1->kind == reg) {
                    if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src2)) {
                        p_basic_var_info p_var = malloc(sizeof(*p_var));
                        *p_var = (basic_var_info) {
                            .var_start = p_vreg1,
                            .basic_var = p_vreg2,
                            .p_step_instr = p_instr,
                            .node = list_head_init(&p_var->node),
                        };
                        list_add_next(&p_var->node, &root->p_var_table);
                        p_vreg2->scev_kind = scev_basic;
                        p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                        *p_vreg2->p_scevexp = (scevexp) {
                            .p_des = p_vreg2,
                            .is_scev1 = false,
                            .is_scev2 = false,
                            .p_var_info = p_var,
                            .p_operand2 = NULL,

                        };
                        flag = true;
                        break;
                    }
                }
                if (p_instr->ir_binary.p_src2->kind == reg) {
                    if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src1)) {
                        p_basic_var_info p_var = malloc(sizeof(*p_var));
                        *p_var = (basic_var_info) {
                            .var_start = p_vreg1,
                            .basic_var = p_vreg2,
                            .p_step_instr = p_instr,
                            .node = list_head_init(&p_var->node),
                        };
                        list_add_next(&p_var->node, &root->p_var_table);
                        p_vreg2->scev_kind = scev_basic;
                        p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                        *p_vreg2->p_scevexp = (scevexp) {
                            .p_des = p_vreg2,
                            .is_scev1 = false,
                            .is_scev2 = false,
                            .p_var_info = p_var,
                            .p_operand2 = NULL,

                        };
                        flag = true;
                        break;
                    }
                }
                // printf("vreg %ld kind %d\n", p_vreg2->id, p_vreg2->scev_kind);
            }
        }
        if (flag) continue;

        list_for_each(p_node, &root->tail_list) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_list_head p_instr_node;
            list_for_each(p_instr_node, &p_basic_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
                if (p_instr->irkind != ir_binary) continue;
                if (p_instr->ir_binary.op != ir_add_op && p_instr->ir_binary.op != ir_sub_op) continue;
                if (p_instr->ir_binary.p_des == p_vreg3) {
                    if (p_instr->ir_binary.p_src1->kind == reg) {
                        if (p_instr->ir_binary.p_src1->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src2)) {
                            p_basic_var_info p_var = malloc(sizeof(*p_var));
                            *p_var = (basic_var_info) {
                                .var_start = p_vreg1,
                                .basic_var = p_vreg2,
                                .p_step_instr = p_instr,
                                .node = list_head_init(&p_var->node),
                            };
                            list_add_next(&p_var->node, &root->p_var_table);
                            p_vreg2->scev_kind = scev_basic;
                            p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                            *p_vreg2->p_scevexp = (scevexp) {
                                .p_des = p_vreg2,
                                .is_scev1 = false,
                                .is_scev2 = false,
                                .p_var_info = p_var,
                                .p_operand2 = NULL,

                            };
                            flag = true;
                            break;
                        }
                    }
                    if (p_instr->ir_binary.p_src2->kind == reg) {
                        if (p_instr->ir_binary.p_src2->p_vreg == p_vreg2 && check_operand(p_instr->ir_binary.p_src1)) {
                            p_basic_var_info p_var = malloc(sizeof(*p_var));
                            *p_var = (basic_var_info) {
                                .var_start = p_vreg1,
                                .basic_var = p_vreg2,
                                .p_step_instr = p_instr,
                                .node = list_head_init(&p_var->node),
                            };
                            list_add_next(&p_var->node, &root->p_var_table);
                            p_vreg2->scev_kind = scev_basic;
                            p_vreg2->p_scevexp = malloc(sizeof(*p_vreg2->p_scevexp));
                            *p_vreg2->p_scevexp = (scevexp) {
                                .p_des = p_vreg2,
                                .is_scev1 = false,
                                .is_scev2 = false,
                                .p_var_info = p_var,
                                .p_operand2 = NULL,

                            };
                            flag = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    //  return;
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
    p_list_head p_node;
    int max_depth = root->head->dom_depth;
    for (int i = root->head->dom_depth; i <= max_depth; ++i) {
        list_for_each(p_node, &root->head->loop_node_list) {
            p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            max_depth = p_block->dom_depth > max_depth ? p_block->dom_depth : max_depth;
            if (p_block->p_nestree_node != root || p_block->dom_depth != i) continue;
            p_list_head p_instr_node;
            list_for_each(p_instr_node, &p_block->instr_list) {
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
                    case ir_mod_op:
                        p_instr->ir_binary.p_des->scev_kind = scev_mod;
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
                            .p_des = p_instr->ir_binary.p_des,
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
                            .p_des = p_instr->ir_binary.p_des,
                            .is_scev1 = true,
                            .is_scev2 = false,
                            .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                            .p_operand2 = p_instr->ir_binary.p_src2,
                        };
                    }
                    else {
                        if (p_instr->ir_binary.p_src1->p_vreg->scev_kind == scev_unknown || p_instr->ir_binary.p_src2->p_vreg->scev_kind == scev_unknown) {
                            if (p_instr->ir_binary.p_src2->p_vreg->scev_kind != scev_unknown && check_operand(p_instr->ir_binary.p_src1)) {
                                p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                                *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                                    .p_des = p_instr->ir_binary.p_des,
                                    .is_scev1 = false,
                                    .is_scev2 = true,
                                    .p_operand1 = p_instr->ir_binary.p_src1,
                                    .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                                };
                            }
                            else if (p_instr->ir_binary.p_src1->p_vreg->scev_kind != scev_unknown && check_operand(p_instr->ir_binary.p_src2)) {
                                p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                                *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                                    .p_des = p_instr->ir_binary.p_des,
                                    .is_scev1 = true,
                                    .is_scev2 = false,
                                    .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                                    .p_operand2 = p_instr->ir_binary.p_src2,
                                };
                            }
                            else
                                p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                        }
                        else {
                            if (p_instr->ir_binary.op == ir_mul_op || p_instr->ir_binary.op == ir_mod_op) {
                                p_instr->ir_binary.p_des->scev_kind = scev_unknown;
                                break;
                            }
                            p_instr->ir_binary.p_des->p_scevexp = malloc(sizeof(*p_instr->ir_binary.p_des->p_scevexp));
                            *p_instr->ir_binary.p_des->p_scevexp = (scevexp) {
                                .p_des = p_instr->ir_binary.p_des,
                                .is_scev1 = true,
                                .is_scev2 = true,
                                .p_scev1 = p_instr->ir_binary.p_src1->p_vreg->p_scevexp,
                                .p_scev2 = p_instr->ir_binary.p_src2->p_vreg->p_scevexp,
                            };
                        }
                    }
                    break;
                }
            }
        }
    }
    // return;
    printf("secv\n");
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

void reduction_var(p_nestedtree_node root, p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target, p_list_head target_list) {
    p_list_head p_instr_node;
    printf("\nreduction var %ld\n", p_basic_block->block_id);
    list_for_each(p_instr_node, &p_basic_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
        if (p_instr->irkind != ir_binary) continue;
        if (p_instr->ir_binary.p_des->scev_kind != scev_mul) continue;
        ir_instr_print(p_instr);

        p_sum_info p_new_info = malloc(sizeof(*p_new_info));
        *p_new_info = (sum_info) {
            .add_const = NULL,
            .add_var = NULL,
            .count = NULL,
            .mul_count = NULL,
            .instr_list = list_head_init(&p_new_info->instr_list),
        };
        p_ir_operand p1, p2;
        if (p_instr->ir_binary.p_des->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_const = ir_vreg_copy(p_instr->ir_binary.p_des);
        p_ir_instr p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_const);
        symbol_func_vreg_add(root->head->p_func, p_new_info->add_const);
        ir_basic_block_addinstr_tail(root->head->p_dom_parent, p_add_instr);
        // list_add_prev(&p_add_instr->node, &p_info->instr_list);
        if (p_instr->ir_binary.p_des->p_type->basic == type_i32)
            p1 = ir_operand_int_gen(0);
        else
            p1 = ir_operand_float_gen(0);
        p_new_info->add_var = ir_vreg_copy(p_instr->ir_binary.p_des);
        p_add_instr = ir_unary_instr_gen(ir_val_assign, p1, p_new_info->add_var);
        symbol_func_vreg_add(root->head->p_func, p_new_info->add_var);
        ir_basic_block_addinstr_tail(root->head->p_dom_parent, p_add_instr);
        if (p_instr->ir_binary.p_des->p_scevexp->is_scev1) {
            scev_add_instr(p_instr->ir_binary.p_src1->p_vreg, p_new_info, root->head->p_dom_parent, false);
            p1 = ir_operand_vreg_gen(p_new_info->add_const);
            p2 = ir_operand_copy(p_instr->ir_binary.p_src2);
            p_new_info->add_const = ir_vreg_copy(p_new_info->add_const);
            instr_insert(ir_mul_op, p1, p2, p_new_info->add_const, root->head->p_dom_parent);
            p1 = ir_operand_vreg_gen(p_new_info->add_var);
            p2 = ir_operand_copy(p_instr->ir_binary.p_src2);
            p_new_info->add_var = ir_vreg_copy(p_new_info->add_var);
            instr_insert(ir_mul_op, p1, p2, p_new_info->add_var, root->head->p_dom_parent);
        }

        else {
            scev_add_instr(p_instr->ir_binary.p_src2->p_vreg, p_new_info, root->head->p_dom_parent, false);
            p1 = ir_operand_vreg_gen(p_new_info->add_const);
            p2 = ir_operand_copy(p_instr->ir_binary.p_src1);
            p_new_info->add_const = ir_vreg_copy(p_new_info->add_const);
            instr_insert(ir_mul_op, p1, p2, p_new_info->add_const, root->head->p_dom_parent);
            p1 = ir_operand_vreg_gen(p_new_info->add_var);
            p2 = ir_operand_copy(p_instr->ir_binary.p_src1);
            p_new_info->add_var = ir_vreg_copy(p_new_info->add_var);
            instr_insert(ir_mul_op, p1, p2, p_new_info->add_var, root->head->p_dom_parent);
        }
        p1 = ir_operand_vreg_gen(p_new_info->add_const);
        p2 = ir_operand_vreg_gen(p_new_info->add_var);
        p_new_info->add_const = ir_vreg_copy(p_new_info->add_const);
        instr_insert(ir_sub_op, p1, p2, p_new_info->add_const, root->head->p_dom_parent);
        p1 = ir_operand_vreg_gen(p_new_info->add_const);
        ir_basic_block_branch_target_add_param(p_target, p1);
        p_ir_vreg p_new_vreg = ir_vreg_copy(p_new_info->add_const);
        symbol_func_vreg_add(root->head->p_func, p_new_vreg);
        ir_basic_block_add_phi(root->head, p_new_vreg);

        p1 = ir_operand_vreg_gen(p_new_vreg);
        p2 = ir_operand_vreg_gen(p_new_info->add_var);
        // p_scevexp p_scevexp = p_instr->ir_binary.p_des->p_scevexp;
        ir_instr_reset_binary(p_instr, ir_add_op, p1, p2, p_instr->ir_binary.p_des);

        p_basic_var_info p_var = malloc(sizeof(*p_var));
        *p_var = (basic_var_info) {
            .var_start = p_new_info->add_const,
            .basic_var = p_new_vreg,
            .p_step_instr = p_instr,
            .node = list_head_init(&p_var->node),
        };
        list_add_next(&p_var->node, &root->p_var_table);
        p1->p_vreg->scev_kind = scev_basic;
        p1->p_vreg->p_scevexp = malloc(sizeof(*p1->p_vreg->p_scevexp));
        *p1->p_vreg->p_scevexp = (scevexp) {
            .p_des = p1->p_vreg,
            .is_scev1 = false,
            .is_scev2 = false,
            .p_var_info = p_var,
            .p_operand2 = NULL,
        };

        p_instr->ir_binary.p_des->p_scevexp->is_scev1 = true;
        p_instr->ir_binary.p_des->p_scevexp->p_scev1 = p1->p_vreg->p_scevexp;
        p_instr->ir_binary.p_des->p_scevexp->is_scev2 = false;
        p_instr->ir_binary.p_des->p_scevexp->p_operand2 = p2;
        p_instr->ir_binary.p_des->p_scevexp->p_des = p_instr->ir_binary.p_des;
        p_instr->ir_binary.p_des->scev_kind = scev_add;

        p_list_head p_list_node;

        list_for_each(p_list_node, &root->p_loop_latch_block->prev_branch_target_list) {
            p_ir_branch_target_node p_branch_target_node = list_entry(p_list_node, ir_branch_target_node, node);
            p_ir_operand p1 = ir_operand_vreg_gen(p_instr->ir_binary.p_des);
            ir_basic_block_branch_target_add_param(p_branch_target_node->p_target, p1);
        }

        p_new_vreg = ir_vreg_copy(p_instr->ir_binary.p_des);
        symbol_func_vreg_add(root->head->p_func, p_new_vreg);
        ir_basic_block_add_phi(root->p_loop_latch_block, p_new_vreg);
        p1 = ir_operand_vreg_gen(p_new_vreg);
        ir_basic_block_branch_target_add_param(root->p_loop_latch_block->p_branch->p_target_1, p1);
        free(p_new_info);
    }
}

void var_strength_reduction(p_nestedtree_node root) {
    if (list_head_alone(&root->p_var_table)) return;
    printf("reduction %ld\n", root->head->block_id);
    int exit_num = 0;
    p_list_head p_node;
    p_ir_basic_block p_exit_block;
    p_ir_basic_block_branch_target p_target;
    list_for_each(p_node, &root->head->prev_branch_target_list) {
        p_ir_branch_target_node p_branch_target_node = list_entry(p_node, ir_branch_target_node, node);
        if (search(root->rbtree->root, (uint64_t) p_branch_target_node->p_target->p_source_block)) continue;
        p_target = p_branch_target_node->p_target;
        break;
    }
    if (root->head->is_loop_exit) ++exit_num;
    list_for_each(p_node, &root->tail_list) {
        if (list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block->is_loop_exit) ++exit_num;
        if (exit_num > 1) return;
    }

    int cnt = 0;
    list_for_each(p_node, &root->loop_exit_block) {
        if (++cnt > 1) return;
    }
    assert(cnt == 1);
    p_ir_basic_block_branch_target p_back_target = NULL;
    list_for_each(p_node, &root->p_loop_latch_block->prev_branch_target_list) {
        if (p_back_target) return;
        p_back_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        p_exit_block = p_back_target->p_source_block;
    }
    assert(p_back_target);
    list_head target_list = list_init_head(&target_list);
    p_ir_basic_block_branch_target p_target1, p_target2;
    p_target1 = root->head->p_branch->p_target_1;
    p_target2 = root->head->p_branch->p_target_2;
    if (p_target1->p_block == root->p_loop_latch_block) {
        p_ir_branch_target_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_branch_target_node) {
            .p_target = p_target1,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &target_list);
    }
    else if (p_target2 && p_target2->p_block == root->p_loop_latch_block) {
        p_ir_branch_target_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_branch_target_node) {
            .p_target = p_target2,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &target_list);
    }

    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_target1 = p_basic_block->p_branch->p_target_1;
        p_target2 = p_basic_block->p_branch->p_target_2;
        if (p_target1->p_block == root->p_loop_latch_block) {
            p_ir_branch_target_node p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_branch_target_node) {
                .p_target = p_target1,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &target_list);
        }
        else if (p_target2 && p_target2->p_block == root->p_loop_latch_block) {
            p_ir_branch_target_node p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_branch_target_node) {
                .p_target = p_target2,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &target_list);
        }
    }
    p_ir_branch_target_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_branch_target_node) {
        .p_target = root->p_loop_latch_block->p_branch->p_target_1,
        .node = list_head_init(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &target_list);
    symbol_func_set_block_id(root->head->p_func);
    reduction_var(root, root->head, p_target, &target_list);
    HashTable *hash = initHashTable(root->head->p_func->block_cnt * 1.2);
    dfs_loop_block(hash, root->head);
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (hashfind(hash, (uint64_t) p_basic_block))
            reduction_var(root, p_basic_block, p_target, &target_list);
    }
    destroyHashTable(hash);
    while (!list_head_alone(&target_list)) {
        p_ir_branch_target_node p_branch_target_node = list_entry(target_list.p_next, ir_branch_target_node, node);
        list_del(&p_branch_target_node->node);
        free(p_branch_target_node);
    }
}

void secv_drop(p_nestedtree_node root) {
    // p_list_head p_node;
    // list_for_each(p_node, &root->son_list) {
    //     p_nestedtree_node p_list_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
    //     secv_drop(p_list_node);
    // }
}
