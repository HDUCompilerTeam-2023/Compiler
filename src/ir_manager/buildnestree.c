#include <ir/basic_block.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <program/print.h>
#include <symbol_gen/func.h>
#include <symbol_print.h>

void ir_build_program_nestedtree(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        symbol_func_set_block_id(p_func);
        ir_build_func_nestedtree(p_func);
    }
    program_ir_nestree_print(p_program);
}

void ir_build_func_nestedtree(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    func_loop_info_drop(p_func);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;
        if (p_true_block) {
            if (p_true_block->p_block == p_basic_block)
                scc_info_target1_gen(p_basic_block, p_true_block->p_block);
            else if (ir_basic_block_dom_check(p_basic_block, p_true_block->p_block))
                scc_info_target1_gen(p_basic_block, p_true_block->p_block);
        }
        if (p_false_block) {
            if (p_false_block->p_block == p_basic_block)
                scc_info_target2_gen(p_basic_block, p_false_block->p_block);
            else if (ir_basic_block_dom_check(p_basic_block, p_false_block->p_block))
                scc_info_target2_gen(p_basic_block, p_false_block->p_block);
        }
    }
    p_nestedtree_node root = malloc(sizeof(*root));

    *root = (nestedtree_node) {
        .head = NULL,
        .parent = NULL,
        .son_list = list_head_init(&root->son_list),
        .tail_list = list_head_init(&root->tail_list),
        .p_var_table = list_head_init(&root->p_var_table),
        .rbtree = initializeRedBlackTree(),
        .p_loop_step = NULL,
        .depth = 0,
        .p_loop_latch_block = NULL,
        .loop_exit_block = list_head_init(&root->loop_exit_block),
        .p_loop_pre_block = NULL,
        .unrolling_time = 1,
        .p_prev_loop = NULL,
    };
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        insert(root->rbtree, (uint64_t) p_basic_block);
    }
    p_func->p_nestedtree_root = root;
    for (int i = 0, flag = true; flag; ++i) {
        flag = false;
        list_for_each(p_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
            if (p_basic_block->nestred_depth != i) continue;
            flag = true;
            if (!p_basic_block->is_loop_head) continue;
            nestedtree_insert(p_basic_block, p_func->p_nestedtree_root);
            p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
            p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;
            if (p_true_block) {
                if (!search(p_basic_block->p_nestree_node->rbtree->root, (uint64_t) p_true_block->p_block))
                    p_basic_block->is_loop_exit = true;
            }
            if (p_false_block) {
                if (!search(p_basic_block->p_nestree_node->rbtree->root, (uint64_t) p_false_block->p_block))
                    p_basic_block->is_loop_exit = true;
            }
        }
    }
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        if (p_basic_block->is_loop_head) continue;
        nestedtree_tail_list_insert(p_basic_block, p_func->p_nestedtree_root);
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;
        if (p_true_block) {
            if (!search(p_basic_block->p_nestree_node->rbtree->root, (uint64_t) p_true_block->p_block))
                p_basic_block->is_loop_exit = true;
        }
        if (p_false_block) {
            if (!search(p_basic_block->p_nestree_node->rbtree->root, (uint64_t) p_false_block->p_block))
                p_basic_block->is_loop_exit = true;
        }
    }
}

void nestedtree_insert(p_ir_basic_block p_basic_block, p_nestedtree_node p_root) {
    p_list_head p_node;
    list_for_each(p_node, &p_root->son_list) {
        p_nestedtree_node p_son_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        if (search(p_son_node->rbtree->root, (uint64_t) p_basic_block)) {
            nestedtree_insert(p_basic_block, p_son_node);
            return;
        }
    }

    p_nestedtree_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (nestedtree_node) {
        .head = p_basic_block,
        .parent = p_root,
        .son_list = list_head_init(&p_new_node->son_list),
        .tail_list = list_head_init(&p_new_node->tail_list),
        .rbtree = initializeRedBlackTree(),
        .p_var_table = list_head_init(&p_new_node->p_var_table),
        .p_loop_step = NULL,
        .depth = p_root->depth + 1,
        .p_loop_latch_block = NULL,
        .loop_exit_block = list_head_init(&p_new_node->loop_exit_block),
        .p_loop_pre_block = NULL,
        .unrolling_time = 1,
        .p_prev_loop = NULL,
    };
    list_for_each(p_node, &p_basic_block->loop_node_list) {
        p_ir_basic_block p_scc_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        insert(p_new_node->rbtree, (uint64_t) p_scc_block);
    }
    p_nested_list_node p_son_node = malloc(sizeof(*p_son_node));
    *p_son_node = (nested_list_node) {
        .p_nested_node = p_new_node,
        .node = list_init_head(&p_son_node->node),
    };
    list_add_next(&p_son_node->node, &p_root->son_list);
    p_basic_block->p_nestree_node = p_new_node;
}

void nestedtree_tail_list_insert(p_ir_basic_block p_basic_block, p_nestedtree_node p_root) {
    p_list_head p_node;
    list_for_each(p_node, &p_root->son_list) {
        p_nestedtree_node p_son_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        if (search(p_son_node->rbtree->root, (uint64_t) p_basic_block)) {
            nestedtree_tail_list_insert(p_basic_block, p_son_node);
            return;
        }
    }
    p_ir_basic_block_list_node p_block_node = malloc(sizeof(*p_block_node));
    *p_block_node = (ir_basic_block_list_node) {
        .p_basic_block = p_basic_block,
        .node = list_head_init(&p_block_node->node),
    };
    list_add_prev(&p_block_node->node, &p_root->tail_list);
    p_basic_block->p_nestree_node = p_root;
}

void func_loop_info_drop(p_symbol_func p_func) {
    p_list_head p_func_list_node;
    list_for_each(p_func_list_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_func_list_node, ir_basic_block, node);
        p_basic_block->nestred_depth = 0;
        p_basic_block->is_loop_back = false;
        p_basic_block->is_loop_exit = false;
        p_basic_block->is_loop_head = false;
        ir_natual_loop_bb_drop(p_basic_block);
    }
    if (p_func->p_nestedtree_root != NULL) nestedtree_node_drop(p_func->p_nestedtree_root);
}

void nestedtree_node_drop(p_nestedtree_node root) {
    p_list_head p_node;
    if (!list_head_alone(&root->son_list)) {
        list_for_each(p_node, &root->son_list) {
            p_nestedtree_node p_son_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
            nestedtree_node_drop(p_son_node);
        }
    }
    while (!list_head_alone(&root->tail_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(root->tail_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&root->son_list)) {
        p_nested_list_node p_list_node = list_entry(root->son_list.p_next, nested_list_node, node);
        list_del(&p_list_node->node);
        free(p_list_node);
    }
    while (!list_head_alone(&root->p_var_table)) {
        p_basic_var_info p_var_info = list_entry(root->p_var_table.p_next, basic_var_info, node);
        list_del(&p_var_info->node);
        free(p_var_info);
    }
    while (!list_head_alone(&root->p_var_table)) {
        p_basic_var_info p_var = list_entry(root->p_var_table.p_next, basic_var_info, node);
        list_del(&p_var->node);
        free(p_var);
    }
    while (!list_head_alone(&root->loop_exit_block)) {
        p_ir_basic_block_list_node p_block_node = list_entry(root->loop_exit_block.p_next, ir_basic_block_list_node, node);
        list_del(&p_block_node->node);
        free(p_block_node);
    }
    destroyRedBlackTree(root->rbtree);
    free(root->p_loop_step);
    free(root);
    root = NULL;
}

void scc_info_target1_gen(p_ir_basic_block p_block, p_ir_basic_block to) {
    if (p_block == to) {
        p_block->is_loop_head = true;
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target1_scc_list);

        if (insert(to->loop_check, (uint64_t) to)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_block,
                .node = list_head_init(&p_new_node->node),
            };
            to->nestred_depth++;
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        return;
    }
    p_block->is_loop_back = true;
    to->is_loop_head = true;
    stack *stk = InitStack();
    stack_push(stk, (uint64_t) p_block);
    RedBlackTree *tree = initializeRedBlackTree();
    insert(tree, (uint64_t) p_block);
    insert(tree, (uint64_t) to);
    while (checkstack(stk)) {
        p_ir_basic_block p_top_block = (p_ir_basic_block) stack_pop(stk);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_top_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target1_scc_list);

        if (insert(to->loop_check, (uint64_t) p_top_block)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_top_block,
                .node = list_head_init(&p_new_node->node),
            };
            p_top_block->nestred_depth++;
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        p_list_head p_node;
        list_for_each(p_node, &p_top_block->prev_branch_target_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            if (insert(tree, (uint64_t) p_prev))
                stack_push(stk, (uint64_t) p_prev);
        }
    }
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = to,
        .node = list_head_init(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &p_block->target1_scc_list);

    if (insert(to->loop_check, (uint64_t) to)) {
        p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = to,
            .node = list_head_init(&p_new_node->node),
        };
        to->nestred_depth++;
        list_add_next(&p_new_node->node, &to->loop_node_list);
    }

    destroystack(stk);
    destroyRedBlackTree(tree);
}

void scc_info_target2_gen(p_ir_basic_block p_block, p_ir_basic_block to) {
    if (p_block == to) {
        p_block->is_loop_head = true;
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target2_scc_list);

        if (insert(to->loop_check, (uint64_t) to)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_block,
                .node = list_head_init(&p_new_node->node),
            };
            to->nestred_depth++;
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }
        return;
    }
    p_block->is_loop_back = true;
    to->is_loop_head = true;
    stack *stk = InitStack();
    stack_push(stk, (uint64_t) p_block);
    RedBlackTree *tree = initializeRedBlackTree();
    insert(tree, (uint64_t) p_block);
    insert(tree, (uint64_t) to);
    while (checkstack(stk)) {
        p_ir_basic_block p_top_block = (p_ir_basic_block) stack_pop(stk);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_top_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target2_scc_list);

        if (insert(to->loop_check, (uint64_t) p_top_block)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_top_block,
                .node = list_head_init(&p_new_node->node),
            };
            p_top_block->nestred_depth++;
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        p_list_head p_node;
        list_for_each(p_node, &p_top_block->prev_branch_target_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            if (insert(tree, (uint64_t) p_prev))
                stack_push(stk, (uint64_t) p_prev);
        }
    }
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = to,
        .node = list_head_init(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &p_block->target2_scc_list);
    if (insert(to->loop_check, (uint64_t) to)) {
        p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = to,
            .node = list_head_init(&p_new_node->node),
        };
        to->nestred_depth++;
        list_add_next(&p_new_node->node, &to->loop_node_list);
    }

    destroystack(stk);
    destroyRedBlackTree(tree);
}

void program_ir_scc_info_print(p_program p_program) {
    printf("\n+++ natural loop print start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        ir_func_scc_info_print(p_func);
    }
    printf("+++ natural loop print end +++\n\n");
}

void program_ir_nestree_print(p_program p_program) {
    printf("\n+++ nested tree print start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        symbol_func_init_print(p_func);
        ir_func_scc_info_print(p_func);
        ir_nestree_print(p_func->p_nestedtree_root, 0);
        printf("\n");
    }
    printf("+++ nested tree print end +++\n\n");
}

void ir_func_scc_info_print(p_symbol_func p_func) {
    symbol_func_init_print(p_func);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;
        if (!list_head_alone(&p_basic_block->target1_scc_list)) {
            printf("back edge %zu->%zu: natural loop { ", p_basic_block->block_id, p_true_block->p_block->block_id);
            p_list_head p_block;
            list_for_each(p_block, &p_basic_block->target1_scc_list) {
                p_ir_basic_block p_scc_block = list_entry(p_block, ir_basic_block_list_node, node)->p_basic_block;
                printf("%zu ", p_scc_block->block_id);
            }
            printf("}\n");
        }
        if (!list_head_alone(&p_basic_block->target2_scc_list)) {
            printf("back edge %zu->%zu: natural loop { ", p_basic_block->block_id, p_false_block->p_block->block_id);
            p_list_head p_block;
            list_for_each(p_block, &p_basic_block->target2_scc_list) {
                p_ir_basic_block p_scc_block = list_entry(p_block, ir_basic_block_list_node, node)->p_basic_block;
                printf("%ld ", p_scc_block->block_id);
            }
            printf("}\n");
        }
    }
    printf("\n");
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        if (!list_head_alone(&p_basic_block->loop_node_list)) {
            printf("head block %ld loop: ", p_basic_block->block_id);
            p_list_head p_block;
            list_for_each(p_block, &p_basic_block->loop_node_list) {
                p_ir_basic_block p_scc_block = list_entry(p_block, ir_basic_block_list_node, node)->p_basic_block;
                printf("%zu ", p_scc_block->block_id);
            }
            printf("\n");
        }
    }
    printf("\n");
}

void ir_nestree_print(p_nestedtree_node p_root, size_t depth) {
    for (size_t i = 0; i < depth; i++)
        printf(" ");
    printf("loop head:");
    if (p_root->head)
        printf(" %ld\n", p_root->head->block_id);
    else
        putchar('\n');
    p_list_head p_node;
    list_for_each(p_node, &p_root->son_list) {
        p_nestedtree_node p_tree_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        ir_nestree_print(p_tree_node, depth + 10);
    }
    for (size_t i = 0; i < depth; i++)
        printf(" ");
    printf("loop tail: ");
    list_for_each(p_node, &p_root->tail_list) {
        p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        printf("%ld ", p_son->block_id);
    }
    printf("\n");
}

void ir_natual_loop_bb_drop(p_ir_basic_block p_basic_block) {
    while (!list_head_alone(&p_basic_block->target1_scc_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->target1_scc_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&p_basic_block->target2_scc_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->target2_scc_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    while (!list_head_alone(&p_basic_block->loop_node_list)) {
        p_ir_basic_block_list_node p_basic_block_list_node = list_entry(p_basic_block->loop_node_list.p_next, ir_basic_block_list_node, node);
        list_del(&p_basic_block_list_node->node);
        free(p_basic_block_list_node);
    }
    clearRedBlackTree(p_basic_block->loop_check);
}
