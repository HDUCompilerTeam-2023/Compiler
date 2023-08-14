#ifndef __IR_MANAGER_BUILDNESTREE__
#define __IR_MANAGER_BUILDNESTREE__

#include <ir.h>
#include <program/def.h>
#include <program/use.h>
#include <symbol.h>
#include <util/structure.h>

typedef struct nested_list_node nested_list_node, *p_nested_list_node;
typedef struct nestedtree_node nestedtree_node, *p_nestedtree_node;
typedef struct basic_var_info basic_var_info, *p_basic_var_info;
typedef struct loop_step_info loop_step_info, *p_loop_step_info;
typedef struct prev_loop prev_loop, *p_prev_loop;

struct prev_loop {
    p_ir_basic_block head;
    list_head tail_list;
    p_ir_instr p_step_add;
    p_ir_instr p_step_mul;
};
struct loop_step_info {
    p_ir_instr p_cond_instr;
    p_basic_var_info p_basic_var;
    bool is_xeq;
};

struct basic_var_info {
    p_ir_vreg var_start;
    p_ir_vreg basic_var;
    p_ir_instr p_step_instr;
    list_head node;
};

struct nestedtree_node {
    p_ir_basic_block head;
    p_nestedtree_node parent;
    list_head tail_list;
    list_head son_list;
    RedBlackTree *rbtree;
    list_head p_var_table;
    p_loop_step_info p_loop_step;
    size_t depth;

    p_ir_basic_block p_loop_pre_block;
    p_ir_basic_block p_loop_latch_block;
    list_head loop_exit_block;

    size_t unrolling_time;
    p_prev_loop p_prev_loop;
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
void func_loop_info_drop(p_symbol_func p_func);
void nestedtree_node_drop(p_nestedtree_node root);

void scc_info_target1_gen(p_ir_basic_block p_block, p_ir_basic_block to);
void scc_info_target2_gen(p_ir_basic_block p_block, p_ir_basic_block to);
void ir_natual_loop_bb_drop(p_ir_basic_block p_basic_block);

void ir_endless_loop_check(p_program p_program);
void endless_loop_check(p_nestedtree_node p_root);

void program_ir_scc_info_print(p_program p_program);
void program_ir_nestree_print(p_program p_program);
#endif