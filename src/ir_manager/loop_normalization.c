#include <ir_manager/loop_normalization.h>

#include <ir.h>
#include <ir_gen.h>
#include <util.h>

#include <ir_print.h>

#include <symbol_gen/func.h>
#include <symbol_gen/type.h>

void program_loop_normalization(p_program p_program) {
    program_var_analysis(p_program, false);
    printf("\nLoop Normalization\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        printf("\n%s\n", p_func->name);
        p_list_head p_son_node;
        list_for_each(p_son_node, &p_func->p_nestedtree_root->son_list) {
            p_nestedtree_node p_list_node = list_entry(p_son_node, nested_list_node, node)->p_nested_node;
            loop_normalization(p_list_node);
        }
        symbol_func_set_block_id(p_func);
    }
}

void loop_normalization(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nestedtree_node p_son_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        loop_normalization(p_son_node);
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_son_node->head->loop_node_list) {
            p_ir_basic_block_list_node p_block_node = list_entry(p_list_node, ir_basic_block_list_node, node);
            p_ir_basic_block p_block = p_block_node->p_basic_block;
            if (!search(root->rbtree->root, (uint64_t) p_block)) {
                insert(root->rbtree, (uint64_t) p_block);
                p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
                *p_new_node = (ir_basic_block_list_node) {
                    .p_basic_block = p_block,
                    .node = list_init_head(&p_new_node->node),
                };
                list_add_prev(&p_new_node->node, &root->head->loop_node_list);
            }
        }
    }
    root->p_loop_pre_block = loop_pre_block_add(root);
    root->p_loop_latch_block = loop_latch_block_add(root);
    loop_exit_block_add(root);
    loop_lcssa(root);
}

p_ir_basic_block ir_basic_block_target_split(p_list_head p_list, p_ir_basic_block p_block, bool is_prev) {
    p_list_head p_node;
    p_ir_basic_block p_new_block = ir_basic_block_gen(p_block->p_func);
    ir_basic_block_insert_prev(p_new_block, p_block);
    ir_basic_block_set_br(p_new_block, p_block);
    list_for_each(p_node, &p_block->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_new_vreg = ir_vreg_copy(p_vreg);
        symbol_func_vreg_add(p_block->p_func, p_new_vreg);
        ir_basic_block_add_phi(p_new_block, p_new_vreg);
        ir_basic_block_branch_target_add_param(p_new_block->p_branch->p_target_1, ir_operand_vreg_gen(p_new_vreg));
    }
    list_for_each(p_node, p_list) {
        p_ir_basic_block_branch_target p_branch_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        ir_basic_block_branch_del_prev_target(p_branch_target);
        p_branch_target->p_block = p_new_block;
        ir_basic_block_add_prev_target(p_branch_target, p_new_block);
    }
    if (p_block->p_func->p_entry_block == p_block)
        p_block->p_func->p_entry_block = p_new_block;
    while (!list_head_alone(p_list)) {
        p_node = p_list->p_next;
        p_ir_branch_target_node p_branch_target_node = list_entry(p_node, ir_branch_target_node, node);
        list_del(p_node);
        free(p_branch_target_node);
    }
    p_nestedtree_node root;
    if (is_prev)
        root = p_block->p_nestree_node->parent;
    else
        root = p_block->p_nestree_node;
    if (root->head) {
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_new_block,
            .node = list_init_head(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &root->head->loop_node_list);
        insert(root->rbtree, (uint64_t) p_new_block);
    }
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = p_new_block,
        .node = list_init_head(&p_new_node->node),
    };
    list_add_prev(&p_new_node->node, &root->tail_list);
    p_new_block->p_nestree_node = root;
    return p_new_block;
}

p_ir_basic_block loop_latch_block_add(p_nestedtree_node root) {
    list_head list = list_init_head(&list);
    p_ir_basic_block_branch_target p_target1, p_target2;
    p_list_head p_node;
    list_for_each(p_node, &root->head->loop_node_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        if (p_block->p_nestree_node != root) continue;
        p_target1 = p_block->p_branch->p_target_1;
        p_target2 = p_block->p_branch->p_target_2;
        if (p_target1->p_block == root->head) {
            p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
            *p_target_node = (ir_branch_target_node) {
                .p_target = p_target1,
                .node = list_init_head(&p_target_node->node),
            };
            list_add_next(&p_target_node->node, &list);
        }
        else if (p_target2 && p_target2->p_block == root->head) {
            p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
            *p_target_node = (ir_branch_target_node) {
                .p_target = p_target2,
                .node = list_init_head(&p_target_node->node),
            };
            list_add_next(&p_target_node->node, &list);
        }
    }
    return ir_basic_block_target_split(&list, root->head, false);
}

static inline void _lcssa_vreg(p_ir_vreg p_vreg, p_nestedtree_node root) {
    p_list_head p_node;
    size_t exit_cnt = 0;
    list_for_each(p_node, &root->loop_exit_block) {
        exit_cnt++;
    }
    assert(exit_cnt);

    p_ir_vreg *exit_phi_map = malloc(sizeof(*exit_phi_map) * exit_cnt);
    memset(exit_phi_map, 0, sizeof(*exit_phi_map) * exit_cnt);

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &p_vreg->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        p_ir_basic_block p_use_bb = NULL;
        switch (p_use->used_type) {
        case instr_ptr:
            p_use_bb = p_use->p_instr->p_basic_block;
            break;
        case bb_param_ptr:
            p_use_bb = p_use->p_bb_param->p_target->p_source_block;
            break;
        case cond_ptr:
        case ret_ptr:
            p_use_bb = p_use->p_basic_block;
            break;
        }
        assert(p_use_bb);
        size_t exit_id = 0;
        p_list_head p_node1, p_next1;
        list_for_each_safe(p_node1, p_next1, &root->loop_exit_block) {
            p_ir_basic_block p_exit = list_entry(p_node1, ir_basic_block_list_node, node)->p_basic_block;
            if (!ir_basic_block_dom_check(p_use_bb, p_exit->p_branch->p_target_1->p_block)) {
                exit_id++;
                continue;
            }
            if (!exit_phi_map[exit_id]) {
                p_ir_vreg p_exit_phi = ir_vreg_gen(symbol_type_copy(p_vreg->p_type));
                symbol_func_vreg_add(p_use_bb->p_func, p_exit_phi);
                ir_basic_block_add_phi(p_exit, p_exit_phi);
                p_list_head p_node;
                list_for_each(p_node, &p_exit->prev_branch_target_list) {
                    p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
                    ir_basic_block_branch_target_add_param(p_target, ir_operand_vreg_gen(p_vreg));
                }
                exit_phi_map[exit_id] = p_exit_phi;
            }
            ir_operand_reset_vreg(p_use, exit_phi_map[exit_id]);
            break;
        }
    }
    free(exit_phi_map);
}

static inline void _lcssa_bb(p_ir_basic_block p_bb, p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &p_bb->basic_block_phis) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        _lcssa_vreg(p_vreg, root);
    }
    list_for_each(p_node, &p_bb->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        p_ir_vreg p_vreg = NULL;
        switch (p_instr->irkind) {
        case ir_binary:
            p_vreg = p_instr->ir_binary.p_des;
            break;
        case ir_unary:
            p_vreg = p_instr->ir_unary.p_des;
            break;
        case ir_gep:
            p_vreg = p_instr->ir_gep.p_des;
            break;
        case ir_call:
            p_vreg = p_instr->ir_call.p_des;
            break;
        case ir_load:
            p_vreg = p_instr->ir_load.p_des;
            break;
        case ir_store:
            break;
        }
        if (!p_vreg)
            continue;

        _lcssa_vreg(p_vreg, root);
    }
}

void loop_lcssa(p_nestedtree_node root) {
    p_list_head p_node;
    _lcssa_bb(root->head, root);
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        _lcssa_bb(p_block, root);
    }
}

p_ir_basic_block loop_pre_block_add(p_nestedtree_node root) {
    list_head list = list_init_head(&list);
    p_list_head p_node;
    int cnt = 0;
    p_ir_basic_block p_ret;
    list_for_each(p_node, &root->head->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_branch_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        assert(p_branch_target->p_block == root->head);
        if (!search(root->rbtree->root, (uint64_t) p_branch_target->p_source_block)) {
            p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
            *p_target_node = (ir_branch_target_node) {
                .p_target = p_branch_target,
                .node = list_init_head(&p_target_node->node),
            };
            list_add_next(&p_target_node->node, &list);
            ++cnt;
            p_ret = p_branch_target->p_source_block;
        }
    }
    if (cnt == 1) {
        while (!list_head_alone(&list)) {
            p_node = (&list)->p_next;
            p_ir_branch_target_node p_branch_target_node = list_entry(p_node, ir_branch_target_node, node);
            list_del(p_node);
            free(p_branch_target_node);
        }
        return p_ret;
    }
    return ir_basic_block_target_split(&list, root->head, true);
}

void loop_exit_block_add(p_nestedtree_node root) {
    p_list_head p_node;
    list_head list = list_init_head(&list);
    p_ir_basic_block_branch_target p_target1 = root->head->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_target2 = root->head->p_branch->p_target_2;
    if (!search(root->rbtree->root, (uint64_t) p_target1->p_block)) {
        p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
        *p_target_node = (ir_branch_target_node) {
            .p_target = p_target1,
            .node = list_init_head(&p_target_node->node),
        };
        list_add_next(&p_target_node->node, &list);
        p_ir_basic_block p_new_block = ir_basic_block_target_split(&list, p_target1->p_block, false);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_new_block,
            .node = list_init_head(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &root->loop_exit_block);
    }
    else if (p_target2 && !search(root->rbtree->root, (uint64_t) p_target2->p_block)) {
        p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
        *p_target_node = (ir_branch_target_node) {
            .p_target = p_target2,
            .node = list_init_head(&p_target_node->node),
        };
        list_add_next(&p_target_node->node, &list);
        p_ir_basic_block p_new_block = ir_basic_block_target_split(&list, p_target2->p_block, false);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_new_block,
            .node = list_init_head(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &root->loop_exit_block);
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_target1 = p_block->p_branch->p_target_1;
        p_target2 = p_block->p_branch->p_target_2;
        if (!search(root->rbtree->root, (uint64_t) p_target1->p_block)) {
            p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
            *p_target_node = (ir_branch_target_node) {
                .p_target = p_target1,
                .node = list_init_head(&p_target_node->node),
            };
            list_add_next(&p_target_node->node, &list);
            p_ir_basic_block p_new_block = ir_basic_block_target_split(&list, p_target1->p_block, false);
            p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_new_block,
                .node = list_init_head(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &root->loop_exit_block);
        }
        else if (p_target2 && !search(root->rbtree->root, (uint64_t) p_target2->p_block)) {
            p_ir_branch_target_node p_target_node = malloc(sizeof(*p_target_node));
            *p_target_node = (ir_branch_target_node) {
                .p_target = p_target2,
                .node = list_init_head(&p_target_node->node),
            };
            list_add_next(&p_target_node->node, &list);
            p_ir_basic_block p_new_block = ir_basic_block_target_split(&list, p_target2->p_block, false);
            p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_new_block,
                .node = list_init_head(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &root->loop_exit_block);
        }
    }
}