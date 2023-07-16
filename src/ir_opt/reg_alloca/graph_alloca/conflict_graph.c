#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>

#include <ir_gen.h>
#include <stdio.h>
#include <symbol_gen.h>

p_graph_node graph_node_gen(p_ir_vreg p_vreg, p_conflict_graph p_graph) {
    p_graph_node p_node = malloc(sizeof(*p_node));
    p_node->p_vreg = p_vreg;
    p_vreg->p_info = p_node;
    p_node->color = -1;
    p_node->p_neighbors = graph_node_list_gen();
    if (p_vreg->if_float) {
        p_node->used_color = malloc(sizeof(*p_node->used_color) * p_graph->reg_num_s);
        memset(p_node->used_color, false, sizeof(*p_node->used_color) * p_graph->reg_num_s);
    }
    else {
        p_node->used_color = malloc(sizeof(*p_node->used_color) * p_graph->reg_num_r);
        memset(p_node->used_color, false, sizeof(*p_node->used_color) * p_graph->reg_num_r);
    }
    p_node->node_id = p_graph->node_num++;
    p_node->seq_id = 0;
    return p_node;
}

void origin_graph_node_gen(p_origin_graph_node p_node, p_ir_vreg p_vreg, p_conflict_graph p_graph) {
    p_node->p_def_node = graph_node_gen(p_vreg, p_graph);
    p_node->p_vmem = NULL;
    p_node->p_spill_node = NULL;
    p_node->p_use_spill_list = graph_node_list_gen();
    p_node->if_pre_color = false;
    p_node->spl_type = none;
    p_node->pcliques = list_head_init(&p_node->pcliques);
    p_node->need_delete = false;
    p_node->in_clique_num = 0;
}

void graph_nodes_init(p_conflict_graph p_graph) {
    p_list_head p_node, p_next;
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        (p_graph->p_nodes + i)->spl_type = none;
        (p_graph->p_nodes + i)->in_clique_num = 0;
        list_for_each_safe(p_node, p_next, &(p_graph->p_nodes + i)->pcliques) {
            p_pclique_node p_pc_node = list_entry(p_node, pclique_node, node);
            list_del(p_node);
            free(p_pc_node);
        }
    }
    graph_node_list_drop(p_graph->seo_seq);
    p_graph->seo_seq = graph_node_list_gen();

    list_for_each_safe(p_node, p_next, &(p_graph->cliques)) {
        p_clique_node p_c_node = list_entry(p_node, clique_node, node);
        graph_node_list_drop(p_c_node->may_spilled_list_r);
        graph_node_list_drop(p_c_node->may_spilled_list_s);
        list_del(p_node);
        free(p_c_node);
    }
}

void graph_node_list_add(p_graph_node_list p_list, p_graph_node p_g_node) {
    p_graph_nodes p_node = malloc(sizeof(*p_node));
    p_node->node = list_head_init(&p_node->node);
    p_node->p_node = p_g_node;
    list_add_prev(&p_node->node, &p_list->node);
    p_list->num++;
}

static inline p_graph_node_list graph_node_list_copy(p_graph_node_list p_list) {
    p_graph_node_list p_copy = malloc(sizeof(*p_copy));
    p_copy->node = list_head_init(&p_copy->node);
    p_list_head p_node;
    list_for_each(p_node, &p_list->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        graph_node_list_add(p_copy, p_g_node);
    }
    return p_copy;
}

p_conflict_graph conflict_graph_gen(size_t reg_num_r, size_t reg_num_s, p_symbol_func p_func) {
    p_conflict_graph p_graph = malloc(sizeof(*p_graph));
    p_graph->reg_num_r = reg_num_r;
    p_graph->reg_num_s = reg_num_s;
    p_graph->p_nodes = NULL;
    p_graph->seo_seq = graph_node_list_gen();
    p_graph->cliques = list_head_init(&p_graph->cliques);
    p_graph->p_func = p_func;
    p_graph->node_num = 0;
    p_list_head p_node;

    size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;

    size_t node_cnt = 0;
    p_graph->p_nodes = malloc(sizeof(*p_graph->p_nodes) * vreg_num);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        origin_graph_node_gen(p_graph->p_nodes + node_cnt++, p_vreg, p_graph);
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        origin_graph_node_gen(p_graph->p_nodes + node_cnt++, p_vreg, p_graph);
    }
    p_graph->origin_node_num = node_cnt;
    return p_graph;
}

p_graph_node_list graph_node_list_gen() {
    p_graph_node_list p_list = malloc(sizeof(*p_list));
    p_list->num = 0;
    p_list->node = list_head_init(&p_list->node);
    return p_list;
}

void add_graph_edge(p_graph_node r1, p_graph_node r2) {
    graph_node_list_add(r1->p_neighbors, r2);
    graph_node_list_add(r2->p_neighbors, r1);
}

void add_reg_graph_edge(p_ir_vreg r1, p_ir_vreg r2) {
    assert(r1 != r2);
    p_graph_node p_node1 = (p_graph_node) r1->p_info;
    p_graph_node p_node2 = (p_graph_node) r2->p_info;
    add_graph_edge(p_node1, p_node2);
}

void neighbors_clear(p_graph_node p_g_node) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_g_node->p_neighbors->node) {
        p_graph_nodes p_nodes = list_entry(p_node, graph_nodes, node);
        node_list_del(p_nodes->p_node->p_neighbors, p_g_node);
        list_del(p_node);
        free(p_nodes);
    }
}

bool if_in_neighbors(p_graph_node p_g_node, p_graph_node p_n_node) {
    p_list_head p_node;
    list_for_each(p_node, &p_g_node->p_neighbors->node) {
        p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
        if (p_neighbor == p_n_node)
            return true;
    }
    return false;
}

void copy_graph_neighbor(p_graph_node p_des, p_graph_node p_src) {
    p_list_head p_node;
    list_for_each(p_node, &p_src->p_neighbors->node) {
        p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
        add_graph_edge(p_des, p_neighbor);
    }
}

void node_list_del(p_graph_node_list p_list, p_graph_node p_del_node) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->node) {
        p_graph_nodes p_n_node = list_entry(p_node, graph_nodes, node);
        if (p_n_node->p_node == p_del_node) {
            list_del(&p_n_node->node);
            free(p_n_node);
            p_list->num--;
            break;
        }
    }
}

p_clique_node clique_node_gen() {
    p_clique_node p_c_node = malloc(sizeof(*p_c_node));
    p_c_node->have_spilled_num_r = p_c_node->have_spilled_num_s = 0;
    p_c_node->may_spilled_list_r = graph_node_list_gen();
    p_c_node->may_spilled_list_s = graph_node_list_gen();
    p_c_node->node = list_head_init(&p_c_node->node);
    p_c_node->spilled_mem_num_r = p_c_node->spilled_mem_num_s = 0;
    return p_c_node;
}

void graph_clique_add(p_conflict_graph p_graph, p_clique_node p_c_node) {
    list_add_prev(&p_c_node->node, &p_graph->cliques);
}
void origin_node_add_pclique(p_origin_graph_node p_o_node, p_clique_node p_c_node) {
    p_pclique_node p_pc_node = malloc(sizeof(*p_pc_node));
    p_pc_node->node = list_head_init(&p_pc_node->node);
    p_pc_node->p_c_node = p_c_node;
    list_add_prev(&p_pc_node->node, &p_o_node->pcliques);
    p_o_node->in_clique_num++;
}
static inline void print_node_list(p_graph_node_list p_list) {
    printf("{ ");
    p_list_head p_node;
    list_for_each(p_node, &p_list->node) {
        p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
        if (p_node->p_next != &p_list->node)
            printf("%ld(%ld), ", p_neighbor->node_id, p_neighbor->p_vreg->id);
        else
            printf("%ld(%ld) ", p_neighbor->node_id, p_neighbor->p_vreg->id);
    }
    printf("}\n");
}

static inline void print_conflict_node(p_graph_node p_g_node) {
    printf("%ld: ", p_g_node->node_id);
    print_node_list(p_g_node->p_neighbors);
}
void print_conflict_graph(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_graph_node p_g_node = (p_graph->p_nodes + i)->p_def_node;
        print_conflict_node(p_g_node);
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            print_conflict_node(p_g_node);
        }
        if ((p_graph->p_nodes + i)->p_spill_node)
            print_conflict_node((p_graph->p_nodes + i)->p_spill_node);
    }
}

void graph_node_list_drop(p_graph_node_list p_list) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_list->node) {
        p_graph_nodes p_nodes = list_entry(p_node, graph_nodes, node);
        list_del(p_node);
        free(p_nodes);
    }
    free(p_list);
}

void graph_node_drop(p_graph_node p_node) {
    graph_node_list_drop(p_node->p_neighbors);
    free(p_node->used_color);
    free(p_node);
}

void conflict_graph_drop(p_conflict_graph p_graph) {
    p_list_head p_node, p_next;
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        graph_node_drop((p_graph->p_nodes + i)->p_def_node);
        list_for_each_safe(p_node, p_next, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_graph_nodes p_g_node = list_entry(p_node, graph_nodes, node);
            graph_node_drop(p_g_node->p_node);
            free(p_g_node);
        }
        free((p_graph->p_nodes + i)->p_use_spill_list);
        list_for_each_safe(p_node, p_next, &(p_graph->p_nodes + i)->pcliques) {
            p_pclique_node p_pc_node = list_entry(p_node, pclique_node, node);
            free(p_pc_node);
        }
        if ((p_graph->p_nodes + i)->p_spill_node)
            graph_node_drop((p_graph->p_nodes + i)->p_spill_node);
    }
    list_for_each_safe(p_node, p_next, &p_graph->cliques) {
        p_clique_node p_c_node = list_entry(p_node, clique_node, node);
        graph_node_list_drop(p_c_node->may_spilled_list_r);
        graph_node_list_drop(p_c_node->may_spilled_list_s);
        free(p_c_node);
    }
    graph_node_list_drop(p_graph->seo_seq);
    free(p_graph->p_nodes);
    free(p_graph);
}

typedef struct node_info {
    p_graph_node p_node;
    size_t label;
    bool visited;
    list_head node;
} node_info, *p_node_info;
static inline void node_info_init(p_node_info seqs_info, p_list_head seq_heads, p_graph_node p_g_node) {
    p_node_info p_info = seqs_info + p_g_node->node_id;
    list_head_init(seq_heads + p_g_node->node_id);
    p_info->p_node = p_g_node;
    p_info->label = 0;
    p_info->visited = false;
    p_info->node = list_head_init(&p_info->node);
    list_add_prev(&p_info->node, seq_heads + 0);
}
// 获得完美消除序列的逆序，也就是着色顺序以及图的色数，可优化
void mcs_get_seqs(p_conflict_graph p_graph) {
    if (p_graph->node_num == 0) {
        p_graph->color_num_r = p_graph->color_num_s = 0;
        return;
    }
    p_list_head seq_heads = malloc(sizeof(*seq_heads) * p_graph->node_num);
    p_node_info seqs_info = malloc(sizeof(*seqs_info) * p_graph->node_num);
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_graph_node p_g_node = (p_graph->p_nodes + i)->p_def_node;
        node_info_init(seqs_info, seq_heads, p_g_node);
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            node_info_init(seqs_info, seq_heads, p_g_node);
        }
        if ((p_graph->p_nodes + i)->p_spill_node)
            node_info_init(seqs_info, seq_heads, (p_graph->p_nodes + i)->p_spill_node);
    }
    size_t max_label = 0;
    size_t current = 0;
    for (size_t i = 0; i < p_graph->node_num; i++) {
        while (list_head_alone(seq_heads + current))
            current--;
        if (max_label < current)
            max_label = current;
        p_node_info p_current_node_info = list_entry((seq_heads + current)->p_next, node_info, node);
        graph_node_list_add(p_graph->seo_seq, p_current_node_info->p_node);
        p_current_node_info->p_node->seq_id = i;
        p_current_node_info->visited = true;
        list_del(&p_current_node_info->node);
        p_list_head p_node;
        list_for_each(p_node, &p_current_node_info->p_node->p_neighbors->node) {
            p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
            p_node_info p_neighbor_info = seqs_info + p_neighbor->node_id;
            if (p_neighbor_info->visited)
                continue;
            p_neighbor_info->label++;
            list_del(&p_neighbor_info->node);
            list_add_next(&p_neighbor_info->node, seq_heads + p_neighbor_info->label);
        }
        current++;
    }
    free(seqs_info);
    free(seq_heads);
}

// 保险起见检验是否是弦图（可删）
void check_chordal(p_conflict_graph p_graph) {
    p_graph_node *p_nodes = malloc(sizeof(void *) * p_graph->node_num);
    size_t num;
    p_list_head p_node;
    list_for_each_tail(p_node, &p_graph->seo_seq->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        num = 0;
        p_list_head p_node;
        list_for_each(p_node, &p_g_node->p_neighbors->node) {
            p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
            if (p_neighbor->seq_id < p_g_node->seq_id) {
                p_nodes[num] = p_neighbor;
                if (p_neighbor->seq_id > p_nodes[0]->seq_id) {
                    p_graph_node tmp = p_nodes[0];
                    p_nodes[0] = p_nodes[num];
                    p_nodes[num] = tmp;
                }
                num++;
            }
        }
        for (size_t i = 1; i < num; i++)
            assert(if_in_neighbors(p_nodes[0], p_nodes[i]));
    }
    free(p_nodes);
}

void set_node_color(p_conflict_graph p_graph, p_graph_node p_node, size_t color) {
    if (p_node->p_vreg->if_float)
        assert(color < p_graph->reg_num_s);
    else
        assert(color < p_graph->reg_num_r);
    p_node->color = color;
    p_node->p_vreg->reg_id = color;
    p_list_head p_neighbor_node;
    list_for_each(p_neighbor_node, &p_node->p_neighbors->node) {
        p_graph_node p_neighbor = list_entry(p_neighbor_node, graph_nodes, node)->p_node;
        if (p_neighbor->p_vreg->if_float == p_node->p_vreg->if_float)
            p_neighbor->used_color[color] = true;
    }
}

// 根据顺序进行着色
void set_graph_color(p_conflict_graph p_graph) {
    assert(p_graph->color_num_r <= p_graph->reg_num_r);
    assert(p_graph->color_num_s <= p_graph->reg_num_s);
    p_list_head p_node;
    list_for_each(p_node, &p_graph->seo_seq->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        if (p_g_node->color != -1) continue;
        size_t lowest = 0;
        while (p_g_node->used_color[lowest])
            lowest++;
        set_node_color(p_graph, p_g_node, lowest);
    }
}

static inline bool if_spill_node(p_conflict_graph p_graph, p_graph_node p_node) {
    return p_node->node_id >= p_graph->origin_node_num || (p_graph->p_nodes + p_node->node_id)->p_vmem
        || (p_graph->p_nodes + p_node->node_id)->p_spill_node;
}

static inline void clique_add_node(p_conflict_graph p_graph, p_clique_node p_c_node, p_graph_node p_g_node) {
    if (if_spill_node(p_graph, p_g_node)) {
        if (p_g_node->p_vreg->if_float)
            p_c_node->have_spilled_num_s++;
        else
            p_c_node->have_spilled_num_r++;
    }
    else {
        if (p_g_node->p_vreg->if_float)
            graph_node_list_add(p_c_node->may_spilled_list_s, p_g_node);
        else
            graph_node_list_add(p_c_node->may_spilled_list_r, p_g_node);
        origin_node_add_pclique(p_graph->p_nodes + p_g_node->node_id, p_c_node);
    }
}

void maximum_clique(p_conflict_graph p_graph) {
    print_node_list(p_graph->seo_seq);
    size_t *tmp_num = malloc(sizeof(*tmp_num) * p_graph->node_num);
    size_t *max_seo_id = malloc(sizeof(*max_seo_id) * p_graph->node_num);
    p_list_head p_node;
    list_for_each(p_node, &p_graph->seo_seq->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        max_seo_id[p_g_node->seq_id] = 0;
        tmp_num[p_g_node->seq_id] = 0;
        p_list_head p_node_;
        list_for_each(p_node_, &p_g_node->p_neighbors->node) {
            p_graph_node p_n_node = list_entry(p_node_, graph_nodes, node)->p_node;
            if (p_n_node->seq_id < p_g_node->seq_id) {
                if (p_n_node->seq_id > max_seo_id[p_g_node->seq_id])
                    max_seo_id[p_g_node->seq_id] = p_n_node->seq_id;
                tmp_num[p_g_node->seq_id]++;
            }
        }
    }

    bool *visited = malloc(sizeof(*visited) * p_graph->node_num);
    memset(visited, false, sizeof(*visited) * p_graph->node_num);
    list_for_each_tail(p_node, &p_graph->seo_seq->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        if (!visited[p_g_node->seq_id]) {
            p_clique_node p_c_node = clique_node_gen();
            clique_add_node(p_graph, p_c_node, p_g_node);
            p_list_head p_node_;
            list_for_each(p_node_, &p_g_node->p_neighbors->node) {
                p_graph_node p_n_node = list_entry(p_node_, graph_nodes, node)->p_node;
                if (p_n_node->seq_id < p_g_node->seq_id)
                    clique_add_node(p_graph, p_c_node, p_n_node);
            }
            graph_clique_add(p_graph, p_c_node);
        }
        if (tmp_num[p_g_node->seq_id] >= tmp_num[max_seo_id[p_g_node->seq_id]] + 1)
            visited[max_seo_id[p_g_node->seq_id]] = true;
    }

    free(tmp_num);
    free(max_seo_id);
    free(visited);
}

void get_color_num(p_conflict_graph p_graph) {
    p_list_head p_node;
    p_graph->color_num_r = 0;
    p_graph->color_num_s = 0;
    list_for_each(p_node, &p_graph->cliques) {
        p_clique_node p_c_node = list_entry(p_node, clique_node, node);
        if (p_c_node->may_spilled_list_r->num + p_c_node->have_spilled_num_r > p_graph->color_num_r)
            p_graph->color_num_r = p_c_node->may_spilled_list_r->num + p_c_node->have_spilled_num_r;
        if (p_c_node->may_spilled_list_s->num + p_c_node->have_spilled_num_s > p_graph->color_num_s)
            p_graph->color_num_s = p_c_node->may_spilled_list_s->num + p_c_node->have_spilled_num_s;
    }
}
double cal_spill_cost(p_origin_graph_node p_o_node) {
    return 1.0 / p_o_node->in_clique_num;
}

static inline int cmp(const void *a, const void *b) {
    double cost1 = cal_spill_cost(*(p_origin_graph_node *) a);
    double cost2 = cal_spill_cost(*(p_origin_graph_node *) b);
    return cost1 > cost2 ? 1 : -1;
}

static inline void print_cliques(p_conflict_graph p_graph) {
    p_list_head p_node;
    list_for_each(p_node, &p_graph->cliques) {
        p_clique_node p_c_node = list_entry(p_node, clique_node, node);
        printf("have spilled r: %ld, may spilled r:", p_c_node->have_spilled_num_r);
        print_node_list(p_c_node->may_spilled_list_r);
        printf("have spilled s: %ld, may spilled s:", p_c_node->have_spilled_num_s);
        print_node_list(p_c_node->may_spilled_list_s);
    }
}

static inline spill_type get_spill_type(p_conflict_graph p_graph, p_origin_graph_node p_o_node) {
    p_list_head p_node;
    size_t max_numr = 0;
    size_t max_nums = 0;
    list_for_each(p_node, &p_o_node->pcliques) {
        p_clique_node p_c_node = list_entry(p_node, pclique_node, node)->p_c_node;
        if (max_numr < p_c_node->have_spilled_num_r + p_c_node->may_spilled_list_r->num + p_c_node->spilled_mem_num_r)
            max_numr = p_c_node->have_spilled_num_r + p_c_node->may_spilled_list_r->num + p_c_node->spilled_mem_num_r;
        if (max_nums < p_c_node->have_spilled_num_s + p_c_node->may_spilled_list_s->num + p_c_node->spilled_mem_num_s)
            max_nums = p_c_node->have_spilled_num_s + p_c_node->may_spilled_list_s->num + p_c_node->spilled_mem_num_s;
    }
    if (p_o_node->p_def_node->p_vreg->if_float) {
        if (max_numr >= p_graph->reg_num_r)
            return reg_mem;
        return reg_reg;
    }
    if (max_nums >= p_graph->reg_num_s)
        return reg_mem;
    return reg_reg;
}

static inline bool is_load_imme_def(p_symbol_func p_func, p_ir_vreg p_vreg) {
    if (p_vreg->is_bb_param) return false;
    if (p_vreg->id < p_func->param_reg_cnt) return false;
    if (p_vreg->p_instr_def->irkind != ir_load) return false;
    if (p_vreg->p_instr_def->ir_load.p_addr->kind != imme) return false;
    return true;
}
static inline void set_vmem(p_conflict_graph p_graph, p_origin_graph_node p_o_node) {
    p_ir_vreg p_src = p_o_node->p_def_node->p_vreg;
    p_symbol_var p_vmem;
    if (is_load_imme_def(p_graph->p_func, p_src)) {
        p_vmem = p_src->p_instr_def->ir_load.p_addr->p_vmem;
        ir_instr_drop(p_o_node->p_def_node->p_vreg->p_instr_def);
        neighbors_clear(p_o_node->p_def_node);
        p_o_node->need_delete = true;
    }
    else {
        p_vmem = symbol_temp_var_gen(symbol_type_copy(p_o_node->p_def_node->p_vreg->p_type));
        symbol_func_add_variable(p_graph->p_func, p_vmem);
        p_o_node->need_delete = false;
    }
    p_o_node->p_vmem = p_vmem;
    p_o_node->spl_type = reg_mem;
}

static inline void set_spill_node(p_conflict_graph p_graph, p_origin_graph_node p_o_node) {
    p_ir_vreg p_src = p_o_node->p_def_node->p_vreg;
    p_ir_vreg p_vreg = ir_vreg_copy(p_src);
    p_vreg->if_float = !p_vreg->if_float;
    symbol_func_vreg_add(p_graph->p_func, p_vreg);
    p_o_node->p_spill_node = graph_node_gen(p_vreg, p_graph);
    p_o_node->spl_type = reg_reg;
    if (is_load_imme_def(p_graph->p_func, p_src)) {
        p_ir_instr p_load = p_src->p_instr_def;
        p_load->ir_load.p_des = p_vreg;
        ir_vreg_set_instr_def(p_vreg, p_load);
        copy_graph_neighbor(p_o_node->p_spill_node, p_o_node->p_def_node);
        neighbors_clear(p_o_node->p_def_node);
        p_o_node->need_delete = true;
    }
    else
        p_o_node->need_delete = false;
}

static void choose_spill_node(p_conflict_graph p_graph, p_graph_node_list p_list, size_t need_spill_num) {
    p_origin_graph_node *may_spilled_list = malloc(sizeof(*may_spilled_list) * p_list->num);
    size_t may_spill_num = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_list->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;
        assert(p_o_node->spl_type == none);
        may_spilled_list[may_spill_num++] = p_o_node;
    }
    qsort(may_spilled_list, may_spill_num, sizeof(*may_spilled_list), cmp);
    p_origin_graph_node *p_2m_list = malloc(sizeof(*p_2m_list) * may_spill_num);
    size_t p_2m_num = 0;
    for (size_t i = 0; i < may_spill_num; i++) {
        if (need_spill_num == 0) break;
        spill_type spl_type = get_spill_type(p_graph, may_spilled_list[i]);
        switch (spl_type) {
        case reg_mem:
            p_2m_list[p_2m_num++] = may_spilled_list[i];
            break;
        case reg_reg:
            set_spill_node(p_graph, may_spilled_list[i]);
            printf("溢出(reg_reg):");
            print_conflict_node(may_spilled_list[i]->p_def_node);
            // 溢出后更新该节点对应得所有极大团的 may_spill
            p_list_head p_pc_node_;
            list_for_each(p_pc_node_, &may_spilled_list[i]->pcliques) {
                p_clique_node p_c = list_entry(p_pc_node_, pclique_node, node)->p_c_node;
                if (may_spilled_list[i]->p_def_node->p_vreg->if_float) {
                    node_list_del(p_c->may_spilled_list_s, may_spilled_list[i]->p_def_node);
                    p_c->have_spilled_num_r++;
                }
                else {
                    node_list_del(p_c->may_spilled_list_r, may_spilled_list[i]->p_def_node);
                    p_c->have_spilled_num_s++;
                }
            }
            need_spill_num--;
            break;
        default:
            assert(0);
            break;
        }
    }
    assert(need_spill_num <= p_2m_num);
    for (size_t i = 0; i < need_spill_num; i++) {
        printf("溢出(mem):");
        print_conflict_node(p_2m_list[i]->p_def_node);
        set_vmem(p_graph, p_2m_list[i]);
        // 溢出后更新该节点对应得所有极大团的 may_spill
        p_list_head p_pc_node_;
        list_for_each(p_pc_node_, &p_2m_list[i]->pcliques) {
            p_clique_node p_c = list_entry(p_pc_node_, pclique_node, node)->p_c_node;
            if (p_2m_list[i]->p_def_node->p_vreg->if_float) {
                node_list_del(p_c->may_spilled_list_s, p_2m_list[i]->p_def_node);
                p_c->spilled_mem_num_s++;
            }
            else {
                node_list_del(p_c->may_spilled_list_r, p_2m_list[i]->p_def_node);
                p_c->spilled_mem_num_r++;
            }
        }
    }
    free(may_spilled_list);
    free(p_2m_list);
}
void choose_spill(p_conflict_graph p_graph) {
    print_cliques(p_graph);
    p_list_head p_node;
    list_for_each(p_node, &p_graph->cliques) {
        p_clique_node p_c_node = list_entry(p_node, clique_node, node);
        if (p_c_node->have_spilled_num_r + p_c_node->may_spilled_list_r->num > p_graph->reg_num_r) {
            assert(p_c_node->have_spilled_num_r <= p_graph->reg_num_r);
            size_t need_spill_num_r = p_c_node->may_spilled_list_r->num + p_c_node->have_spilled_num_r - p_graph->reg_num_r;
            choose_spill_node(p_graph, p_c_node->may_spilled_list_r, need_spill_num_r);
        }
        if (p_c_node->have_spilled_num_s + p_c_node->may_spilled_list_s->num > p_graph->reg_num_s) {
            assert(p_c_node->have_spilled_num_s <= p_graph->reg_num_s);
            size_t need_spill_num_s = p_c_node->may_spilled_list_s->num + p_c_node->have_spilled_num_s - p_graph->reg_num_s;
            choose_spill_node(p_graph, p_c_node->may_spilled_list_s, need_spill_num_s);
        }
    }
}

typedef struct ou_unit ou_unit, *p_ou_unit;
typedef struct ou_unit_queue ou_unit_queue, *p_ou_unit_queue;
typedef struct ou_unit_list ou_unit_list, *p_ou_unit_list;

typedef enum unit_type unit_type;
enum unit_type {
    single,
    range
};
struct ou_unit {
    p_graph_node_list p_nodes;
    list_head node;
    size_t *nodes_color;
    p_bitmap candidates;
    size_t color;
    bool if_float;
};
struct ou_unit_queue {
    list_head ou_units;
    p_graph_node_list p_nodes;
    list_head node;
    unit_type type;
    size_t allowed_color;
    bool if_float;
    size_t node_num;
    size_t *nodes_color;
};

struct ou_unit_list {
    list_head node;
    p_bitmap pinned;
    size_t *nodes_color;
    size_t node_num;
    size_t reg_num_r;
    size_t reg_num_s;
    bool *if_visited;
};
static inline p_ou_unit create_ou_unit(size_t color, p_graph_node_list p_nodes, p_ou_unit_queue p_unit_queue) {
    p_ou_unit p_unit = malloc(sizeof(*p_unit));
    p_unit->if_float = p_unit_queue->if_float;
    p_unit->candidates = bitmap_gen(p_unit_queue->node_num);
    bitmap_set_empty(p_unit->candidates);
    p_unit->node = list_head_init(&p_unit->node);
    p_unit->nodes_color = malloc(sizeof(p_unit->nodes_color) * p_unit_queue->node_num);
    memcpy(p_unit->nodes_color, p_unit_queue->nodes_color, sizeof(*p_unit_queue->nodes_color) * p_unit_queue->node_num);
    p_unit->p_nodes = p_nodes;
    p_unit->color = color;
    list_add_prev(&p_unit->node, &p_unit_queue->ou_units);
    return p_unit;
}
static inline p_ou_unit_queue create_ou_unit_queue(bool if_float, unit_type type, size_t allowed_color, p_ou_unit_list p_list) {
    p_ou_unit_queue p_unit_queue = malloc(sizeof(*p_unit_queue));
    p_unit_queue->node = list_head_init(&p_unit_queue->node);
    p_unit_queue->p_nodes = graph_node_list_gen();
    p_unit_queue->type = type;
    p_unit_queue->allowed_color = allowed_color;
    p_unit_queue->if_float = if_float;
    p_unit_queue->node_num = p_list->node_num;
    p_unit_queue->nodes_color = malloc(sizeof(*p_unit_queue->nodes_color) * p_unit_queue->node_num);
    p_unit_queue->ou_units = list_head_init(&p_unit_queue->ou_units);
    list_add_prev(&p_unit_queue->node, &p_list->node);
    return p_unit_queue;
}

static inline p_ou_unit_list ou_unit_list_gen(p_conflict_graph p_graph) {
    p_ou_unit_list p_list = malloc(sizeof(*p_list));
    p_list->node = list_head_init(&p_list->node);
    p_list->pinned = bitmap_gen(p_graph->node_num);
    bitmap_set_empty(p_list->pinned);

    p_list->node_num = p_graph->node_num;
    p_list->nodes_color = malloc(sizeof(*p_list->nodes_color) * p_graph->node_num);
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_list->nodes_color[(p_graph->p_nodes + i)->p_def_node->node_id] = (p_graph->p_nodes + i)->p_def_node->color;
        if ((p_graph->p_nodes + i)->p_spill_node)
            p_list->nodes_color[(p_graph->p_nodes + i)->p_spill_node->node_id] = (p_graph->p_nodes + i)->p_spill_node->color;
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            p_list->nodes_color[p_g_node->node_id] = p_g_node->color;
        }
    }
    p_list->reg_num_r = p_graph->reg_num_r;
    p_list->reg_num_s = p_graph->reg_num_s;

    p_list->if_visited = malloc(sizeof(*p_list->if_visited) * p_list->node_num);
    return p_list;
}

static inline void ou_unit_drop(p_ou_unit p_unit) {
    list_del(&p_unit->node);
    graph_node_list_drop(p_unit->p_nodes);
    free(p_unit->nodes_color);
    bitmap_drop(p_unit->candidates);
    free(p_unit);
}

static inline void ou_unit_queue_drop(p_ou_unit_queue p_unit_queue) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_unit_queue->ou_units) {
        p_ou_unit p_unit = list_entry(p_node, ou_unit, node);
        ou_unit_drop(p_unit);
    }
    list_del(&p_unit_queue->node);
    graph_node_list_drop(p_unit_queue->p_nodes);
    free(p_unit_queue->nodes_color);
    free(p_unit_queue);
}

static inline void ou_unit_list_drop(p_ou_unit_list p_unit_list) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_unit_list->node) {
        p_ou_unit_queue p_unit_queue = list_entry(p_node, ou_unit_queue, node);
        ou_unit_queue_drop(p_unit_queue);
    }
    bitmap_drop(p_unit_list->pinned);
    free(p_unit_list->nodes_color);
    free(p_unit_list->if_visited);
    free(p_unit_list);
}

static inline void deal_block_target(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target, p_ou_unit_list p_list) {
    if (!p_target) return;
    if (p_target->p_block != p_basic_block) return;
    p_list_head p_node_param;
    p_list_head p_node_phi = p_basic_block->basic_block_phis->bb_phi.p_prev;
    p_list_head p_node = p_list->node.p_prev;
    list_for_each_tail(p_node_param, &p_target->block_param) {
        p_ir_operand p_param = list_entry(p_node_param, ir_bb_param, node)->p_bb_param;
        if (p_param->kind == reg) {
            p_ir_vreg p_phi = list_entry(p_node_phi, ir_bb_phi, node)->p_bb_phi;
            if (p_phi->if_float == p_param->p_vreg->if_float) {
                p_ou_unit p_unit = list_entry(p_node, ou_unit, node);
                graph_node_list_add(p_unit->p_nodes, (p_graph_node) p_param->p_vreg->p_info);
            }
        }
        p_node_phi = p_node_phi->p_prev;
        p_node = p_node->p_prev;
    }
}

static inline void create_ou_unit_phi(p_ir_basic_block p_basic_block, p_ou_unit_list p_list) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        p_ou_unit_queue p_unit_queue;
        if (p_phi->if_float)
            p_unit_queue = create_ou_unit_queue(p_phi->if_float, range, p_list->reg_num_s, p_list);
        else
            p_unit_queue = create_ou_unit_queue(p_phi->if_float, range, p_list->reg_num_r, p_list);
        graph_node_list_add(p_unit_queue->p_nodes, (p_graph_node) (p_phi->p_info));
    }

    p_list_head p_prev_node;
    list_for_each(p_prev_node, &p_basic_block->prev_basic_block_list) {
        p_ir_basic_block p_prev_block = list_entry(p_prev_node, ir_basic_block_list_node, node)->p_basic_block;
        deal_block_target(p_basic_block, p_prev_block->p_branch->p_target_1, p_list);
        deal_block_target(p_basic_block, p_prev_block->p_branch->p_target_2, p_list);
    }
}

static inline void create_ou_unit_func_param(p_symbol_func p_func, p_ou_unit_list p_list) {
    p_list_head p_node;
    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
        if (p_param->if_float) {
            p_ou_unit_queue p_queue = create_ou_unit_queue(true, single, s, p_list);
            graph_node_list_add(p_queue->p_nodes, (p_graph_node) p_param->p_info);
            s++;
        }
        else {
            p_ou_unit_queue p_queue = create_ou_unit_queue(false, single, r, p_list);
            graph_node_list_add(p_queue->p_nodes, (p_graph_node) p_param->p_info);
            r++;
        }
    }
}

static inline void create_ou_unit_func_call(p_ir_call_instr p_call_instr, p_ou_unit_list p_list) {
    p_list_head p_node;
    size_t r = 0;
    size_t s = 0;
    list_for_each(p_node, &p_call_instr->param_list) {
        p_ir_param p_param = list_entry(p_node, ir_param, node);
        if (p_param->is_in_mem)
            continue;
        p_ir_operand p_param_operand = p_param->p_param;
        bool if_float = (p_param_operand->p_type->ref_level == 0 && p_param_operand->p_type->basic == type_f32);
        if (if_float) {
            if (p_param_operand->kind == reg) {
                assert(p_param_operand->p_vreg->if_float == if_float);
                p_ou_unit_queue p_queue = create_ou_unit_queue(true, single, s, p_list);
                graph_node_list_add(p_queue->p_nodes, (p_graph_node) p_param_operand->p_vreg->p_info);
            }
            s++;
        }
        else {
            if (p_param_operand->kind == reg) {
                assert(p_param_operand->p_vreg->if_float == if_float);
                p_ou_unit_queue p_queue = create_ou_unit_queue(false, single, r, p_list);
                graph_node_list_add(p_queue->p_nodes, (p_graph_node) p_param_operand->p_vreg->p_info);
            }
            r++;
        }
    }
}
typedef struct ret_info ret_info, *p_ret_info;
typedef enum try_type try_type;
enum try_type {
    ok,
    candidate,
    pinned,
    forbidden,
};
struct ret_info {
    try_type type;
    p_graph_node p_g_node;
};
static inline ret_info try_color(p_graph_node p_g_node, size_t color, p_ou_unit p_unit, p_ou_unit_list p_list) {
    if (p_list->if_visited[p_g_node->node_id])
        return (ret_info) { ok, NULL };
    p_list->if_visited[p_g_node->node_id] = true;
    size_t origin_color = p_unit->nodes_color[p_g_node->node_id];
    if (origin_color == color)
        if (p_unit->if_float == p_g_node->p_vreg->if_float)
            return (ret_info) { ok, NULL };
    if (bitmap_if_in(p_list->pinned, p_g_node->node_id))
        return (ret_info) { pinned, p_g_node };
    if (bitmap_if_in(p_unit->candidates, p_g_node->node_id))
        return (ret_info) { candidate, p_g_node };
    // maybe forbidden
    p_list_head p_node;
    list_for_each(p_node, &p_g_node->p_neighbors->node) {
        p_graph_node p_n_node = list_entry(p_node, graph_nodes, node)->p_node;
        if (p_n_node->p_vreg->if_float != p_unit->if_float)
            continue;
        if (p_unit->nodes_color[p_n_node->node_id] != color)
            continue;
        ret_info ret = try_color(p_n_node, origin_color, p_unit, p_list);
        if (ret.type != ok)
            return ret;
    }
    p_unit->nodes_color[p_g_node->node_id] = color;
    return (ret_info) { ok, NULL };
}

static inline void deal_unit_queue(p_ou_unit_queue p_unit_queue, p_ou_unit_list p_list) {
    while (!list_head_alone(&p_unit_queue->ou_units)) {
        p_ou_unit p_unit = list_entry(p_unit_queue->ou_units.p_next, ou_unit, node);
        p_list_head p_node;
        bool if_success = true;
        list_for_each(p_node, &p_unit->p_nodes->node) {
            p_graph_node p_phi_node = list_entry(p_unit->p_nodes->node.p_next, graph_nodes, node)->p_node;
            p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            memset(p_list->if_visited, false, sizeof(*p_list->if_visited) * p_list->node_num);
            ret_info ret = try_color(p_g_node, p_unit->color, p_unit, p_list);
            if (ret.type == ok)
                bitmap_add_element(p_unit->candidates, p_g_node->node_id);
            else {
                if (p_g_node != p_phi_node) {
                    if (ret.type == candidate && ret.p_g_node != p_phi_node) {
                        p_graph_node_list p_new_nodes1 = graph_node_list_copy(p_unit->p_nodes);
                        node_list_del(p_new_nodes1, p_g_node);
                        create_ou_unit(p_unit->color, p_new_nodes1, p_unit_queue);
                        p_graph_node_list p_new_nodes2 = graph_node_list_copy(p_unit->p_nodes);
                        node_list_del(p_new_nodes2, ret.p_g_node);
                        create_ou_unit(p_unit->color, p_new_nodes2, p_unit_queue);
                    }
                    else {
                        p_graph_node_list p_new_nodes = graph_node_list_copy(p_unit->p_nodes);
                        node_list_del(p_new_nodes, p_g_node);
                        create_ou_unit(p_unit->color, p_new_nodes, p_unit_queue);
                    }
                }
                if_success = false;
                break;
            }
        }
        if (if_success) {
            memcpy(p_unit_queue->nodes_color, p_unit->nodes_color, sizeof(*p_list->nodes_color) * p_list->node_num);
            bitmap_merge_not_new(p_list->pinned, p_unit->candidates);
            return;
        }
        ou_unit_drop(p_unit);
    }
}

static inline void reset_node_color(p_conflict_graph p_graph, p_graph_node p_g_node, size_t color) {
    if (p_g_node->color == color) return;
    // print_conflict_node(p_g_node);
    // printf("    reset:%ld -> %ld\n", p_g_node->color, color);
    set_node_color(p_graph, p_g_node, color);
}
static inline void reset_graph_color(p_conflict_graph p_graph, p_ou_unit_list p_list) {
    size_t use_reg_num_r = 0;
    size_t use_reg_num_s = 0;
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_origin_graph_node p_o_node = p_graph->p_nodes + i;
        size_t color = p_list->nodes_color[p_o_node->p_def_node->node_id];
        if (p_o_node->p_def_node->p_vreg->if_float)
            use_reg_num_s = (color + 1) > use_reg_num_s ? (color + 1) : use_reg_num_s;
        else
            use_reg_num_r = (color + 1) > use_reg_num_r ? (color + 1) : use_reg_num_r;
        reset_node_color(p_graph, p_o_node->p_def_node, color);
        p_list_head p_node;
        list_for_each(p_node, &p_o_node->p_use_spill_list->node) {
            p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            color = p_list->nodes_color[p_g_node->node_id];
            if (p_g_node->p_vreg->if_float)
                use_reg_num_s = (color + 1) > use_reg_num_s ? (color + 1) : use_reg_num_s;
            else
                use_reg_num_r = (color + 1) > use_reg_num_r ? (color + 1) : use_reg_num_r;
            reset_node_color(p_graph, p_g_node, color);
        }
        if (p_o_node->p_spill_node) {
            color = p_list->nodes_color[p_o_node->p_spill_node->node_id];
            if (p_o_node->p_spill_node->p_vreg->if_float)
                use_reg_num_s = (color + 1) > use_reg_num_s ? (color + 1) : use_reg_num_s;
            else
                use_reg_num_r = (color + 1) > use_reg_num_r ? (color + 1) : use_reg_num_r;
            reset_node_color(p_graph, p_o_node->p_spill_node, color);
        }
    }
    p_graph->p_func->use_reg_num_r = use_reg_num_r;
    p_graph->p_func->use_reg_num_s = use_reg_num_s;
}
void adjust_graph_color(p_conflict_graph p_graph) {
    p_ou_unit_list p_list = ou_unit_list_gen(p_graph);

    create_ou_unit_func_param(p_graph->p_func, p_list);

    p_list_head p_block_node;
    list_for_each(p_block_node, &p_graph->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        create_ou_unit_phi(p_basic_block, p_list);
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind == ir_call) {
                create_ou_unit_func_call(&p_instr->ir_call, p_list);
                if (p_instr->ir_call.p_des)
                    create_ou_unit_queue(p_instr->ir_call.p_func->ret_type == type_f32, single, 0, p_list);
            }
        }
        if (p_basic_block->p_branch->kind == ir_ret_branch && p_basic_block->p_branch->p_exp) {
            bool if_float = p_graph->p_func->ret_type == type_f32;
            create_ou_unit_queue(if_float, single, 0, p_list);
        }
    }
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_list->node) {
        p_ou_unit_queue p_unit_queue = list_entry(p_node, ou_unit_queue, node);
        memcpy(p_unit_queue->nodes_color, p_list->nodes_color, sizeof(*p_unit_queue->nodes_color) * p_unit_queue->node_num);
        switch (p_unit_queue->type) {
        case single:
            create_ou_unit(p_unit_queue->allowed_color, graph_node_list_copy(p_unit_queue->p_nodes), p_unit_queue);
            break;
        case range:
            for (size_t i = 0; i < p_unit_queue->allowed_color; i++) {
                create_ou_unit(i, graph_node_list_copy(p_unit_queue->p_nodes), p_unit_queue);
            }
            break;
        }
        deal_unit_queue(p_unit_queue, p_list);
        memcpy(p_list->nodes_color, p_unit_queue->nodes_color, sizeof(*p_list->nodes_color) * p_list->node_num);
        ou_unit_queue_drop(p_unit_queue);
    }
    reset_graph_color(p_graph, p_list);
    ou_unit_list_drop(p_list);
}