#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>

#include <ir/vreg.h>

#include <stdio.h>

p_graph_node graph_node_gen(p_ir_vreg p_vreg, size_t reg_num, size_t node_id) {
    p_graph_node p_node = malloc(sizeof(*p_node));
    p_node->p_vreg = p_vreg;
    p_vreg->p_info = p_node;
    p_node->color = -1;
    p_node->p_neighbors = graph_node_list_gen();
    p_node->used_color = malloc(sizeof(*p_node->used_color) * reg_num);
    p_node->node_id = node_id;
    p_node->seq_id = 0;
    return p_node;
}

void origin_graph_node_gen(p_origin_graph_node p_node, p_ir_vreg p_vreg, size_t reg_num, size_t node_id) {
    p_node->p_def_node = graph_node_gen(p_vreg, reg_num, node_id);
    p_node->p_vmem = NULL;
    p_node->p_use_spill_list = graph_node_list_gen();
    p_node->if_pre_color = false;
}

void graph_nodes_init(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        (p_graph->p_nodes + i)->if_need_spill = false;
        (p_graph->p_nodes + i)->in_clique_num = 0;
    }
    free(p_graph->seo_seq);
    p_graph->seo_seq = malloc(sizeof(*p_graph->seo_seq) * p_graph->node_num);
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

p_conflict_graph conflict_graph_gen(size_t node_num, p_origin_graph_node p_nodes, size_t reg_num) {
    p_conflict_graph p_graph = malloc(sizeof(*p_graph));
    p_graph->node_num = p_graph->origin_node_num = node_num;
    p_graph->reg_num = reg_num;
    p_graph->p_nodes = p_nodes;
    p_graph->seo_seq = NULL;
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
    if (r1->if_float == r2->if_float)
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
    }
}

void graph_node_list_drop(p_graph_node_list p_list) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_list->node) {
        list_del(p_node);
        p_graph_nodes p_nodes = list_entry(p_node, graph_nodes, node);
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
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        graph_node_drop((p_graph->p_nodes + i)->p_def_node);
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_graph_nodes p_g_node = list_entry(p_node, graph_nodes, node);
            graph_node_drop(p_g_node->p_node);
            free(p_g_node);
        }
        free((p_graph->p_nodes + i)->p_use_spill_list);
    }
    free(p_graph->p_nodes);
    free(p_graph->seo_seq);
    free(p_graph);
}

// 获得完美消除序列的逆序，也就是着色顺序以及图的色数，可优化
void mcs_get_seqs(p_conflict_graph p_graph) {
    if (p_graph->node_num == 0) {
        p_graph->color_num = 0;
        return;
    }
    typedef struct node_info {
        p_graph_node p_node;
        size_t label;
        bool visited;
        list_head node;
    } node_info, *p_node_info;
    p_list_head seq_heads = malloc(sizeof(*seq_heads) * p_graph->node_num);
    p_node_info seqs_info = malloc(sizeof(*seqs_info) * p_graph->node_num);
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_graph_node p_g_node = (p_graph->p_nodes + i)->p_def_node;
        list_head_init(seq_heads + p_g_node->node_id);
        (seqs_info + p_g_node->node_id)->p_node = p_g_node;
        (seqs_info + p_g_node->node_id)->label = 0;
        (seqs_info + p_g_node->node_id)->visited = false;
        (seqs_info + p_g_node->node_id)->node = list_head_init(&(seqs_info + p_g_node->node_id)->node);
        list_add_prev(&(seqs_info + p_g_node->node_id)->node, seq_heads + 0);
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->p_use_spill_list->node) {
            p_g_node = list_entry(p_node, graph_nodes, node)->p_node;
            list_head_init(seq_heads + p_g_node->node_id);
            (seqs_info + p_g_node->node_id)->p_node = p_g_node;
            (seqs_info + p_g_node->node_id)->label = 0;
            (seqs_info + p_g_node->node_id)->visited = false;
            (seqs_info + p_g_node->node_id)->node = list_head_init(&(seqs_info + p_g_node->node_id)->node);
            list_add_prev(&(seqs_info + p_g_node->node_id)->node, seq_heads + 0);
        }
    }
    size_t max_label = 0;
    size_t current = 0;
    for (size_t i = 0; i < p_graph->node_num; i++) {
        while (list_head_alone(seq_heads + current))
            current--;
        if (max_label < current)
            max_label = current;
        p_node_info p_current_node_info = list_entry((seq_heads + current)->p_next, node_info, node);
        p_graph->seo_seq[i] = p_current_node_info->p_node;
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
    // 得到色数(为 最大的 label + 1)
    p_graph->color_num = max_label + 1;
    free(seqs_info);
    free(seq_heads);
}

// 保险起见检验是否是弦图（可删）
void check_chordal(p_conflict_graph p_graph) {
    bool *visited = malloc(sizeof(*visited) * p_graph->node_num);
    memset(visited, false, sizeof(*visited) * p_graph->node_num);
    for (size_t i = p_graph->node_num - 1; i < p_graph->node_num; i--) {
        visited[p_graph->seo_seq[i]->node_id] = true;
        p_list_head p_node;
        p_graph_node p_min_node = NULL;
        list_for_each(p_node, &p_graph->seo_seq[i]->p_neighbors->node) {
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
    assert(color < p_graph->reg_num);
    p_node->color = color;
    p_node->p_vreg->reg_id = color;
    p_list_head p_neighbor_node;
    list_for_each(p_neighbor_node, &p_node->p_neighbors->node) {
        p_graph_node p_neighbor = list_entry(p_neighbor_node, graph_nodes, node)->p_node;
        p_neighbor->used_color[color] = true;
    }
}

// 根据顺序进行着色
void set_graph_color(p_conflict_graph p_graph) {
    assert(p_graph->color_num <= p_graph->reg_num);
    for (size_t i = 0; i < p_graph->node_num; i++) {
        p_graph_node p_node = p_graph->seo_seq[i];
        if (p_node->color != -1) continue;
        size_t lowest = 0;
        while (p_node->used_color[lowest])
            lowest++;
        set_node_color(p_graph, p_node, lowest);
    }
}

static inline bool if_spill_node(p_conflict_graph p_graph, p_graph_node p_node) {
    return p_node->node_id >= p_graph->origin_node_num || (p_graph->p_nodes + p_node->node_id)->p_vmem;
}

void maximum_clique_spill(p_conflict_graph p_graph) {
    p_graph_node **cliques = malloc(sizeof(*cliques) * p_graph->node_num);
    size_t *clique_num = malloc(sizeof(*clique_num) * p_graph->node_num);
    for (size_t i = 0; i < p_graph->node_num; i++) {
        cliques[i] = malloc(sizeof(*cliques[i]) * p_graph->node_num); // 节点邻居且在左边（成团）的节点
        clique_num[i] = 1; // 团的数量
        p_graph_node p_g_node = p_graph->seo_seq[i];
        cliques[i][0] = p_g_node;
        cliques[i][1] = p_graph->seo_seq[0];
        p_list_head p_node;
        list_for_each(p_node, &p_g_node->p_neighbors->node) {
            p_graph_node p_n_node = list_entry(p_node, graph_nodes, node)->p_node;
            if (p_n_node->seq_id < p_g_node->seq_id) {
                cliques[i][clique_num[i]] = p_n_node;
                // 第一个为最右边的节点
                if (p_n_node->seq_id > cliques[i][1]->seq_id) {
                    p_graph_node tmp = cliques[i][clique_num[i]];
                    cliques[i][clique_num[i]] = cliques[i][1];
                    cliques[i][1] = tmp;
                }
                clique_num[i]++;
            }
        }
    }

    size_t *need_spill_clique = malloc(sizeof(*need_spill_clique) * p_graph->node_num);
    size_t need_spill_clique_num = 0;
    bool *visited = malloc(sizeof(*visited) * p_graph->node_num);
    memset(visited, false, sizeof(*visited) * p_graph->node_num);
    for (size_t i = p_graph->node_num - 1; i < p_graph->node_num; i--) {
        if (!visited[i]) {
            if (clique_num[i] > p_graph->reg_num) {
                for (size_t j = 0; j < clique_num[i]; j++) {
                    if (!if_spill_node(p_graph, cliques[i][j]))
                        (p_graph->p_nodes + cliques[i][j]->node_id)->in_clique_num++;
                }
                need_spill_clique[need_spill_clique_num++] = i;
            }
        }
        if (clique_num[i] >= clique_num[cliques[i][1]->seq_id] + 1)
            visited[cliques[i][1]->seq_id] = true;
    }

    // 选择溢出
    for (size_t i = 0; i < need_spill_clique_num; i++) {
        size_t current_clique = need_spill_clique[i];
        size_t may_spill_num = 0;
        size_t have_spilled_num = 0;
        p_origin_graph_node *may_spill = malloc(sizeof(*may_spill) * (clique_num[current_clique] + 1));
        for (size_t j = 0; j < clique_num[current_clique]; j++) {
            if (if_spill_node(p_graph, cliques[current_clique][j])) {
                have_spilled_num++;
                continue;
            }
            p_origin_graph_node p_o_node = (p_graph->p_nodes + cliques[current_clique][j]->node_id);
            if (p_o_node->if_need_spill) continue;
            may_spill[may_spill_num++] = p_o_node;
        }
        if (have_spilled_num + may_spill_num <= p_graph->reg_num) {
            free(may_spill);
            continue;
        }
        assert(have_spilled_num <= p_graph->reg_num);
        size_t need_spill_num = have_spilled_num + may_spill_num - p_graph->reg_num;
        // TODO: sort spill costs
        for (size_t j = 0; j < need_spill_num; j++) {
            may_spill[j]->if_need_spill = true;
        }
        free(may_spill);
    }
    free(clique_num);
    for (size_t i = 0; i < p_graph->node_num; i++)
        free(cliques[i]);
    free(cliques);
    free(need_spill_clique);
    free(visited);
}
