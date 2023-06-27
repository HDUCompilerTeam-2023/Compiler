#ifndef __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__
#define __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__

#include <ir.h>
typedef struct graph_node graph_node, *p_graph_node;
typedef struct origin_graph_node origin_graph_node, *p_origin_graph_node;
typedef struct spill_node spill_node, *p_spill_node;
typedef struct neighbor_node neighbor_node, *p_neighbor_node;
typedef struct conflict_graph conflict_graph, *p_conflict_graph;

struct neighbor_node {
    p_graph_node p_neighbor;
    list_head node;
};

struct spill_node {
    list_head node;
    p_graph_node p_spill;
};

struct origin_graph_node {
    bool if_pre_color;
    bool if_need_spill;
    p_graph_node p_def_node;
    p_symbol_var p_vmem;
    list_head use_spill_list;
};

struct graph_node {
    p_ir_vreg p_vreg;
    size_t color;
    size_t node_id;
    list_head neighbors;
    bool *used_color;
};

struct conflict_graph {
    p_origin_graph_node p_nodes; // 节点列表
    size_t node_num; // 节点数量
    size_t origin_node_num; // 未溢出前的节点数量
    p_graph_node *seo_seq; // 完美消除序列
    size_t color_num; // 冲突图的色数
    size_t reg_num; // 可用寄存器数量
};

void origin_graph_node_gen(p_origin_graph_node p_node, p_ir_vreg p_vreg, size_t reg_num, size_t node_id);
p_graph_node graph_node_gen(p_ir_vreg p_vreg, size_t reg_num, size_t node_id);
void graph_nodes_init(p_conflict_graph p_graph);
p_neighbor_node graph_neighbor_node_gen(p_graph_node p_node);
void spill_list_add(p_origin_graph_node p_s_node, p_graph_node p_g_node);
void add_graph_edge_at(p_graph_node node, p_list_head p_next);
void add_graph_edge(p_graph_node r1, p_graph_node r2);
void add_reg_graph_edge(p_ir_vreg r1, p_ir_vreg r2);
bool if_in_neighbors(p_graph_node p_g_node, p_graph_node p_n_node);
void node_neighbor_del(p_graph_node p_g_node, p_graph_node p_del_node);
p_conflict_graph conflict_graph_gen(size_t node_num, p_origin_graph_node p_nodes, size_t reg_num);
void print_conflict_graph(p_conflict_graph p_graph);
void conflict_graph_drop(p_conflict_graph p_graph);

void mcs_get_seqs(p_conflict_graph p_graph);
void set_graph_color(p_conflict_graph p_graph);
void set_node_color(p_conflict_graph p_graph, p_graph_node p_node, size_t color);
void check_chordal(p_conflict_graph p_graph);

#endif