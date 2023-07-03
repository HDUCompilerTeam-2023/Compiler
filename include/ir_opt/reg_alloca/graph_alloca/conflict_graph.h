#ifndef __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__
#define __IR_OPT_REG_ALLOC_GRAPH_ALLOCA_CONFLICT_GRAPH__

#include <ir.h>
typedef struct graph_node graph_node, *p_graph_node;
typedef struct origin_graph_node origin_graph_node, *p_origin_graph_node;
typedef struct graph_nodes graph_nodes, *p_graph_nodes;
typedef struct graph_node_list graph_node_list, *p_graph_node_list;
typedef struct conflict_graph conflict_graph, *p_conflict_graph;

typedef struct clique_node clique_node, *p_clique_node;
typedef struct pclique_node pclique_node, *p_pclique_node;
typedef struct pclique_list pclique_list, *p_pclique_list;

typedef enum spill_type spill_type;
enum spill_type {
    none,
    reg_reg,
    reg_mem,
};
struct graph_nodes {
    p_graph_node p_node;
    list_head node;
};

struct graph_node_list {
    list_head node;
    size_t num;
};

struct clique_node {
    list_head node;
    size_t have_spilled_num_r;
    size_t have_spilled_num_s;
    p_graph_node_list may_spilled_list_r;
    p_graph_node_list may_spilled_list_s;
};

struct pclique_node {
    p_clique_node p_c_node;
    list_head node;
};

struct origin_graph_node {
    bool if_pre_color;
    spill_type spl_type;

    size_t in_clique_num;
    list_head pcliques;

    p_graph_node p_def_node;

    p_symbol_var p_vmem;
    p_graph_node p_spill_node;

    p_graph_node_list p_use_spill_list;
};

struct graph_node {
    p_ir_vreg p_vreg;
    size_t color;
    size_t node_id;
    size_t seq_id;
    p_graph_node_list p_neighbors;
    bool *used_color;
};

struct conflict_graph {
    p_origin_graph_node p_nodes; // 节点列表
    size_t node_num; // 节点数量
    size_t origin_node_num; // 未溢出前的节点数量
    p_graph_node_list seo_seq; // 完美消除序列
    size_t color_num_r; // 冲突图的色数
    size_t color_num_s; // 冲突图的色数
    size_t reg_num_r; // 可用普通寄存器数量
    size_t reg_num_s; // 可用浮点寄存器数量

    p_symbol_func p_func;
    list_head cliques;
};

void origin_graph_node_gen(p_origin_graph_node p_node, p_ir_vreg p_vreg, p_conflict_graph p_graph);
p_graph_node graph_node_gen(p_ir_vreg p_vreg, p_conflict_graph p_graph);
p_graph_node_list graph_node_list_gen();
void graph_nodes_init(p_conflict_graph p_graph);
void graph_node_list_add(p_graph_node_list p_list, p_graph_node p_g_node);
void add_graph_edge(p_graph_node r1, p_graph_node r2);
void add_reg_graph_edge(p_ir_vreg r1, p_ir_vreg r2);
bool if_in_neighbors(p_graph_node p_g_node, p_graph_node p_n_node);
void node_list_del(p_graph_node_list p_list, p_graph_node p_del_node);
p_conflict_graph conflict_graph_gen(size_t reg_num_r, size_t reg_num_s, p_symbol_func p_func);
void conflict_graph_set_nodes(p_conflict_graph p_graph, p_origin_graph_node p_nodes, size_t num);
void print_conflict_graph(p_conflict_graph p_graph);
void graph_node_list_drop(p_graph_node_list p_list);
void conflict_graph_drop(p_conflict_graph p_graph);

void mcs_get_seqs(p_conflict_graph p_graph);
void set_graph_color(p_conflict_graph p_graph);
void set_node_color(p_conflict_graph p_graph, p_graph_node p_node, size_t color);
void check_chordal(p_conflict_graph p_graph);

p_clique_node clique_node_gen();
void origin_node_add_pclique(p_origin_graph_node p_o_node, p_clique_node p_c_node);
void graph_clique_add(p_conflict_graph p_graph, p_clique_node p_c_node);
void maximum_clique(p_conflict_graph p_graph);
void get_color_num(p_conflict_graph p_graph);
void choose_spill(p_conflict_graph p_graph);
#endif