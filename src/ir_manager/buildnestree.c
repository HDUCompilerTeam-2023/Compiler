#include <ir/basic_block.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/buildnestree.h>
#include <program/print.h>
#include <symbol_gen/func.h>
#include <symbol_print.h>

void ir_build_program_nestedtree(p_program p_program) {
    program_nestedtree_drop(p_program);
    ir_set_program_scc(p_program);
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
    p_nestedtree_node root = malloc(sizeof(*root));
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        if (!p_basic_block->block_id) {
            assert(!p_basic_block->is_loop_head);
            *root = (nestedtree_node) {
                .head = p_basic_block,
                .parent = NULL,
                .son_list = list_head_init(&root->son_list),
                .tail_list = list_head_init(&root->tail_list),
                .rbtree = initializeRedBlackTree(),
                .depth = 0,
            };
            for (int i = 0; i < p_func->block_cnt; ++i)
                insert(root->rbtree, i);
            p_func->p_nestedtree_root = root;
            p_basic_block->p_nestree_node = root;
            continue;
        }
        if (p_basic_block->is_loop_head)
            nestedtree_insert(p_basic_block, p_func->p_nestedtree_root);
        else if (p_basic_block->dom_depth)
            nestedtree_tail_list_insert(p_basic_block, p_func->p_nestedtree_root);
        else {
            p_basic_block->p_nestree_node = p_func->p_nestedtree_root;
            p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_basic_block,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &p_func->p_nestedtree_root->tail_list);
        }
    }
}

void nestedtree_insert(p_ir_basic_block p_basic_block, p_nestedtree_node p_root) {
    p_list_head p_head = &p_basic_block->loop_node_list;
    p_list_head p_node;

    list_for_each(p_node, &p_root->son_list) {
        p_nestedtree_node p_son_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        if (search(p_son_node->rbtree->root, p_basic_block->block_id)) {
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
        .depth = p_root->depth + 1,
    };
    list_for_each(p_node, p_head) {
        p_ir_basic_block p_scc_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        insert(p_new_node->rbtree, p_scc_block->block_id);
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
        if (search(p_son_node->rbtree->root, p_basic_block->block_id)) {
            nestedtree_tail_list_insert(p_basic_block, p_son_node);
            return;
        }
    }
    p_ir_basic_block_list_node p_block_node = malloc(sizeof(*p_block_node));
    *p_block_node = (ir_basic_block_list_node) {
        .p_basic_block = p_basic_block,
        .node = list_head_init(&p_block_node->node),
    };
    list_add_next(&p_block_node->node, &p_root->tail_list);
    p_basic_block->p_nestree_node = p_root;
}

void program_nestedtree_drop(p_program p_program) {
    program_naturaloop_drop(p_program);
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_nestedtree_node p_del = list_entry(p_node, symbol_func, node)->p_nestedtree_root;
        if (p_del != NULL) nestedtree_node_drop(p_del);
    }
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
    destroyRedBlackTree(root->rbtree);
    free(root);
    root = NULL;
}

void ir_set_program_scc(p_program p_program) {
    program_naturaloop_drop(p_program);
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        ir_cfg_set_func_scc(p_func);
    }
    program_ir_scc_info_print(p_program);
}

void ir_cfg_set_func_scc(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;

        if (p_true_block && p_true_block->p_block->block_id == p_basic_block->block_id) {
            printf("init\n");
            scc_info_target1_gen(p_basic_block, p_true_block->p_block);
        }

        else if (p_true_block && ir_basic_block_dom_check(p_basic_block, p_true_block->p_block)) {
            printf("init\n");
            scc_info_target1_gen(p_basic_block, p_true_block->p_block);
        }
        if (p_false_block && p_false_block->p_block->block_id == p_basic_block->block_id)
            scc_info_target2_gen(p_basic_block, p_false_block->p_block);
        else if (p_false_block && ir_basic_block_dom_check(p_basic_block, p_false_block->p_block)) {
            scc_info_target2_gen(p_basic_block, p_false_block->p_block);
        }
    }
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

        if (insert(to->loop_check, to->block_id)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_block,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        return;
    }
    p_block->is_loop_tail = true;
    to->is_loop_head = true;
    stack *stk = InitStack();
    stack_push(stk, p_block);
    RedBlackTree *tree = initializeRedBlackTree();
    insert(tree, p_block->block_id);
    insert(tree, to->block_id);
    while (checkstack(stk)) {
        p_ir_basic_block p_top_block = stack_pop(stk);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_top_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target1_scc_list);

        if (insert(to->loop_check, p_top_block->block_id)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_top_block,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        p_list_head p_node;
        list_for_each(p_node, &p_top_block->prev_branch_target_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            if (insert(tree, p_prev->block_id))
                stack_push(stk, p_prev);
        }
    }
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = to,
        .node = list_head_init(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &p_block->target1_scc_list);

    if (insert(to->loop_check, to->block_id)) {
        p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = to,
            .node = list_head_init(&p_new_node->node),
        };
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

        if (insert(to->loop_check, to->block_id)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_block,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }
        return;
    }
    p_block->is_loop_tail = true;
    to->is_loop_head = true;
    stack *stk = InitStack();
    stack_push(stk, p_block);
    RedBlackTree *tree = initializeRedBlackTree();
    insert(tree, p_block->block_id);
    insert(tree, to->block_id);
    while (checkstack(stk)) {
        p_ir_basic_block p_top_block = stack_pop(stk);
        p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = p_top_block,
            .node = list_head_init(&p_new_node->node),
        };
        list_add_next(&p_new_node->node, &p_block->target2_scc_list);

        if (insert(to->loop_check, p_top_block->block_id)) {
            p_new_node = malloc(sizeof(*p_new_node));
            *p_new_node = (ir_basic_block_list_node) {
                .p_basic_block = p_top_block,
                .node = list_head_init(&p_new_node->node),
            };
            list_add_next(&p_new_node->node, &to->loop_node_list);
        }

        p_list_head p_node;
        list_for_each(p_node, &p_top_block->prev_branch_target_list) {
            p_ir_basic_block p_prev = list_entry(p_node, ir_branch_target_node, node)->p_target->p_source_block;
            if (insert(tree, p_prev->block_id))
                stack_push(stk, p_prev);
        }
    }
    p_ir_basic_block_list_node p_new_node = malloc(sizeof(*p_new_node));
    *p_new_node = (ir_basic_block_list_node) {
        .p_basic_block = to,
        .node = list_head_init(&p_new_node->node),
    };
    list_add_next(&p_new_node->node, &p_block->target2_scc_list);
    if (insert(to->loop_check, to->block_id)) {
        p_new_node = malloc(sizeof(*p_new_node));
        *p_new_node = (ir_basic_block_list_node) {
            .p_basic_block = to,
            .node = list_head_init(&p_new_node->node),
        };
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
            printf("loop: ");
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
    printf("loop head: %ld\n", p_root->head->block_id);
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

void program_naturaloop_drop(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        p_list_head p_func_list_node;
        list_for_each(p_func_list_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_func_list_node, ir_basic_block, node);
            ir_natual_loop_bb_drop(p_basic_block);
        }
    }
}

stack *InitStack() {
    stack *stk = (stack *) malloc(sizeof(stack));
    stk->stackdata = (p_ir_basic_block *) malloc(32 * sizeof(p_ir_basic_block));
    if (!stk->stackdata) {
        return NULL;
    }
    stk->stacktop = -1;
    stk->stacksize = 32;
    return stk;
}

void stack_push(stack *stk, p_ir_basic_block p_basic_block) {
    if (++stk->stacktop == stk->stacksize) {
        stk->stacksize <<= 1;
        stk->stackdata = (p_ir_basic_block *) realloc(stk->stackdata, sizeof(p_ir_basic_block) * stk->stacksize);
    }
    stk->stackdata[stk->stacktop] = p_basic_block;
}

p_ir_basic_block stack_pop(stack *stk) {
    if (stk->stacktop == -1) return NULL;
    return stk->stackdata[stk->stacktop--];
}

p_ir_basic_block stack_top(stack *stk) {
    if (stk->stacktop == -1) return NULL;
    return stk->stackdata[stk->stacktop--];
}

bool checkstack(stack *stk) {
    if (stk->stacktop == -1) return false;
    return true;
}

void destroystack(stack *stk) {
    free(stk->stackdata);
    free(stk);
}