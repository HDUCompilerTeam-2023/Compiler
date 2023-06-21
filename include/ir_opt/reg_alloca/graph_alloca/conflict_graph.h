#ifndef __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__
#define __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__

#include <ir.h>
typedef struct graph_node graph_node, *p_graph_node;
typedef struct neighbor_node neighbor_node, *p_neighbor_node;
typedef struct conflict_graph conflict_graph, *p_conflict_graph;

struct neighbor_node {
    p_graph_node p_neighbor;
    list_head node;
};

struct graph_node {
    p_ir_vreg p_vreg;
    size_t color;
    list_head neighbors;
};

struct conflict_graph {
    p_graph_node p_nodes; // 节点列表
    size_t node_num; // 节点数量
    p_graph_node *seo_seq; // 完美消除序列
    size_t color_num; // 冲突图的色数
    size_t reg_num; // 可用寄存器数量
    size_t *map; // vreg 编号到图的节点编号的映射
};

void graph_node_gen(p_graph_node p_node, p_ir_vreg p_vreg);
p_neighbor_node graph_neighbor_node_gen(p_graph_node p_node);
void add_graph_edge(p_graph_node r1, p_graph_node r2);
bool if_in_neighbors(p_graph_node p_g_node, p_graph_node p_n_node);
p_conflict_graph conflict_graph_gen(size_t node_num, size_t *map, p_graph_node p_nodes, size_t reg_num);
void print_conflict_graph(p_conflict_graph p_graph);
void conflict_graph_drop(p_conflict_graph p_graph);
#endif