#ifndef __IR_MANAGER_BUILDNESTREE__
#define __IR_MANAGER_BUILDNESTREE__

#include <ir.h>
#include <program/def.h>
#include <program/use.h>
#include <symbol.h>
#include <util/structure.h>

typedef struct nested_list_node nested_list_node, *p_nested_list_node;
typedef struct nestedtree_node nestedtree_node, *p_nestedtree_node;

struct nestedtree_node {
    p_ir_basic_block head;
    p_nestedtree_node parent;
    list_head tail_list;
    list_head son_list;
    RedBlackTree *rbtree;
    size_t depth;
};

struct nested_list_node {
    p_nestedtree_node p_nested_node;
    list_head node;
};

void ir_build_program_nestedtree(p_program p_program);
void program_ir_scc_info_print(p_program p_program);
void program_ir_nestree_print(p_program p_program);
void ir_func_scc_info_print(p_symbol_func p_func);
void ir_nestree_print(p_nestedtree_node p_root, size_t depth);
void ir_build_func_nestedtree(p_symbol_func p_func);
void nestedtree_insert(p_ir_basic_block p_basic_block, p_nestedtree_node p_root);
void nestedtree_tail_list_insert(p_ir_basic_block p_basic_block, p_nestedtree_node root);
void program_nestedtree_drop(p_program p_program);
void nestedtree_node_drop(p_nestedtree_node root);

typedef struct {
    int stacktop;
    int stacksize;

    p_ir_basic_block *stackdata;
} stack;

stack *InitStack();
void stack_push(stack *pushstack, p_ir_basic_block pushnumber);
p_ir_basic_block stack_pop(stack *popstack);
p_ir_basic_block stack_top(stack *popstack);
bool checkstack(stack *checkstack);
void destroystack(stack *stk);

void ir_set_program_scc(p_program p_program);
void ir_cfg_set_func_scc(p_symbol_func p_func);
void scc_info_target1_gen(p_ir_basic_block p_block, p_ir_basic_block to);
void scc_info_target2_gen(p_ir_basic_block p_block, p_ir_basic_block to);
void program_naturaloop_drop(p_program p_program);
void ir_natual_loop_bb_drop(p_ir_basic_block p_basic_block);

void program_ir_scc_info_print(p_program p_program);
void program_ir_nestree_print(p_program p_program);
#endif