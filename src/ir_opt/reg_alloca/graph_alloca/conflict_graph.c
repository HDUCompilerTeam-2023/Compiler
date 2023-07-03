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
}

void graph_nodes_init(p_conflict_graph p_graph) {
    p_list_head p_node, p_next;
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        (p_graph->p_nodes + i)->spl_type = none;
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

p_list_head get_node_pos(p_graph_node_list p_list, p_graph_node p_g_node) {
    p_list_head p_head = &p_list->node;
    p_list_head p_node = p_head;
    list_for_each(p_node, p_head) {
        p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
        if (p_neighbor->node_id > p_g_node->node_id)
            return p_node;
        if (p_neighbor->node_id == p_g_node->node_id)
            break;
    }
    if (p_node == p_head) {
        return p_node;
    }
    return NULL;
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

    p_graph->p_nodes = malloc(sizeof(*p_graph->p_nodes) * vreg_num);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        origin_graph_node_gen(p_graph->p_nodes + p_vreg->id, p_vreg, p_graph);
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        origin_graph_node_gen(p_graph->p_nodes + p_vreg->id, p_vreg, p_graph);
    }
    p_graph->origin_node_num = vreg_num;
    return p_graph;
}

p_graph_node_list graph_node_list_gen() {
    p_graph_node_list p_list = malloc(sizeof(*p_list));
    p_list->num = 0;
    p_list->node = list_head_init(&p_list->node);
    return p_list;
}

void add_node_list_at(p_graph_node_list p_list, p_graph_node node, p_list_head p_next) {
    p_graph_nodes p_neighbor = malloc(sizeof(*p_neighbor));
    p_neighbor->node = list_head_init(&p_neighbor->node);
    p_neighbor->p_node = node;
    list_add_prev(&p_neighbor->node, p_next);
    p_list->num++;
}

void add_graph_edge(p_graph_node r1, p_graph_node r2) {
    p_list_head p_pos1 = get_node_pos(r1->p_neighbors, r2);
    if (p_pos1)
        add_node_list_at(r1->p_neighbors, r2, p_pos1);

    p_list_head p_pos2 = get_node_pos(r2->p_neighbors, r1);
    if (p_pos2)
        add_node_list_at(r2->p_neighbors, r1, p_pos2);
}

void add_reg_graph_edge(p_ir_vreg r1, p_ir_vreg r2) {
    assert(r1 != r2);
    p_graph_node p_node1 = (p_graph_node) r1->p_info;
    p_graph_node p_node2 = (p_graph_node) r2->p_info;
    add_graph_edge(p_node1, p_node2);
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
            printf("%ld, ", p_neighbor->node_id);
        else
            printf("%ld ", p_neighbor->node_id);
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
    bool *visited = malloc(sizeof(*visited) * p_graph->node_num);
    memset(visited, false, sizeof(*visited) * p_graph->node_num);
    p_list_head p_node;
    list_for_each_tail(p_node, &p_graph->seo_seq->node) {
        p_graph_node p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
        visited[p_g_node->node_id] = true;
        p_list_head p_node;
        p_graph_node p_min_node = NULL;
        list_for_each(p_node, &p_g_node->p_neighbors->node) {
            p_graph_node p_neighbor = list_entry(p_node, graph_nodes, node)->p_node;
            if (!visited[p_neighbor->node_id]) {
                if (!p_min_node)
                    p_min_node = p_neighbor;
                else // 最小的节点与其他节点必定相邻
                    assert(if_in_neighbors(p_min_node, p_neighbor));
            }
        }
    }
    free(visited);
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
        if (max_numr < p_c_node->have_spilled_num_r + p_c_node->may_spilled_list_r->num)
            max_numr = p_c_node->have_spilled_num_r + p_c_node->may_spilled_list_r->num;
        if (max_nums < p_c_node->have_spilled_num_s + p_c_node->may_spilled_list_s->num)
            max_nums = p_c_node->have_spilled_num_s + p_c_node->may_spilled_list_s->num;
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

static inline void set_vmem(p_conflict_graph p_graph, p_origin_graph_node p_o_node) {
    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_o_node->p_def_node->p_vreg->p_type));
    symbol_func_add_variable(p_graph->p_func, p_vmem);
    p_o_node->p_vmem = p_vmem;
    p_o_node->spl_type = reg_mem;
}

static inline void set_spill_node(p_conflict_graph p_graph, p_origin_graph_node p_o_node) {
    p_ir_vreg p_vreg = ir_vreg_copy(p_o_node->p_def_node->p_vreg);
    p_vreg->if_float = !p_vreg->if_float;
    symbol_func_vreg_add(p_graph->p_func, p_vreg);
    p_o_node->p_spill_node = graph_node_gen(p_vreg, p_graph);
    p_o_node->spl_type = reg_reg;
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
            // 溢出后更新该节点对应得所有极大团的 may_spill
            p_list_head p_pc_node_;
            list_for_each(p_pc_node_, &may_spilled_list[i]->pcliques) {
                p_clique_node p_c = list_entry(p_pc_node_, pclique_node, node)->p_c_node;
                node_list_del(p_c->may_spilled_list_r, may_spilled_list[i]->p_def_node);
                p_c->have_spilled_num_s++;
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
        set_vmem(p_graph, p_2m_list[i]);
        // 溢出后更新该节点对应得所有极大团的 may_spill
        p_list_head p_pc_node_;
        list_for_each(p_pc_node_, &p_2m_list[i]->pcliques) {
            p_clique_node p_c = list_entry(p_pc_node_, pclique_node, node)->p_c_node;
            node_list_del(p_c->may_spilled_list_r, p_2m_list[i]->p_def_node);
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
