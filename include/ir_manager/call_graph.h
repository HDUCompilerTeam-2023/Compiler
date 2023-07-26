#ifndef __IR_MANAGER_CALL_GRAPH__
#define __IR_MANAGER_CALL_GRAPH__

#include <program/use.h>
#include <symbol.h>
#include <ir.h>

typedef struct call_graph_node call_graph_node, *p_call_graph_node;
typedef struct call_graph_edge call_graph_edge, *p_call_graph_edge;
typedef struct call_instr_node call_instr_node, *p_call_instr_node;

struct call_graph_node {
    p_symbol_func p_func;
    size_t caller_cnt, callee_cnt;
    list_head caller, callee;
    bool is_recursion;
};

struct call_graph_edge {
    p_call_graph_node p_caller, p_callee;
    list_head caller_node, callee_node;

    size_t call_instr_cnt;
    list_head call_instr;
};

struct call_instr_node {
    p_call_graph_edge p_cg_edge;
    p_ir_instr p_call_instr;
    list_head node;
};

void ir_call_instr_node_gen(p_ir_instr p_instr);
void ir_call_instr_node_drop(p_ir_instr p_instr);

void ir_build_call_graph(p_program p_ir);
void ir_call_graph_print(p_program p_ir);

void ir_call_graph_node_gen(p_symbol_func p_func);
void ir_call_graph_node_drop(p_symbol_func);

#endif
