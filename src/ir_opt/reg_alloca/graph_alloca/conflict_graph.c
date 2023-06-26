#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>

#include <ir/vreg.h>

#include <stdio.h>

p_graph_node graph_node_gen(p_ir_vreg p_vreg, size_t reg_num, size_t node_id) {
    p_graph_node p_node = malloc(sizeof(*p_node));
    p_node->p_vreg = p_vreg;
    p_vreg->p_info = p_node;
    p_node->color = -1;
    p_node->neighbors = list_head_init(&p_node->neighbors);
    p_node->used_color = malloc(sizeof(*p_node->used_color) * reg_num);
    p_node->node_id = node_id;
    return p_node;
}

void origin_graph_node_gen(p_origin_graph_node p_node, p_ir_vreg p_vreg, size_t reg_num, size_t node_id) {
    p_node->p_def_node = graph_node_gen(p_vreg, reg_num, node_id);
    p_node->p_vmem = NULL;
    p_node->use_spill_list = list_head_init(&p_node->use_spill_list);
    p_node->if_pre_color = false;
}

void graph_nodes_init(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        memset((p_graph->p_nodes + i)->p_def_node->used_color, false, sizeof(*(p_graph->p_nodes + i)->p_def_node->used_color) * p_graph->reg_num);
        p_list_head p_node;
        list_for_each(p_node, &p_graph->p_nodes[i].use_spill_list) {
            p_graph_node p_g_node = list_entry(p_node, spill_node, node)->p_spill;
            memset(p_g_node->used_color, false, sizeof(*p_g_node->used_color) * p_graph->reg_num);
        }
        (p_graph->p_nodes + i)->if_need_spill = false;
    }
    free(p_graph->seo_seq);
    p_graph->seo_seq = malloc(sizeof(*p_graph->seo_seq) * p_graph->node_num);
}

void spill_list_add(p_origin_graph_node p_o_node, p_graph_node p_g_node) {
    p_spill_node p_s_node = malloc(sizeof(*p_s_node));
    p_s_node->node = list_head_init(&p_s_node->node);
    p_s_node->p_spill = p_g_node;
    list_add_prev(&p_s_node->node, &p_o_node->use_spill_list);
}

p_neighbor_node graph_neighbor_node_gen(p_graph_node p_node) {
    p_neighbor_node p_neighbor = malloc(sizeof(*p_neighbor));
    p_neighbor->node = list_head_init(&p_neighbor->node);
    p_neighbor->p_neighbor = p_node;
    return p_neighbor;
}

p_conflict_graph conflict_graph_gen(size_t node_num, p_origin_graph_node p_nodes, size_t reg_num) {
    p_conflict_graph p_graph = malloc(sizeof(*p_graph));
    p_graph->node_num = p_graph->origin_node_num = node_num;
    p_graph->reg_num = reg_num;
    p_graph->p_nodes = p_nodes;
    p_graph->seo_seq = NULL;
    return p_graph;
}

void add_graph_edge_at(p_graph_node node, p_list_head p_next) {
    p_neighbor_node p_neighbor = malloc(sizeof(*p_neighbor));
    p_neighbor->node = list_head_init(&p_neighbor->node);
    p_neighbor->p_neighbor = node;
    list_add_prev(&p_neighbor->node, p_next);
}

void add_graph_edge(p_graph_node r1, p_graph_node r2) {
    p_list_head p_head = &(r2)->neighbors;
    p_list_head p_node = p_head;
    list_for_each(p_node, p_head) {
        p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
        if (p_neighbor->node_id > r1->node_id) {
            add_graph_edge_at(r1, p_node);
            break;
        }
        if (p_neighbor->node_id == r1->node_id)
            break;
    }
    if (p_node == p_head) {
        add_graph_edge_at(r1, p_node);
    }

    p_head = &(r1)->neighbors;
    list_for_each(p_node, p_head) {
        p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
        if (p_neighbor->node_id > r2->node_id) {
            add_graph_edge_at(r2, p_node);
            break;
        }
        if (p_neighbor->node_id == r2->node_id)
            break;
    }
    if (p_node == p_head) {
        add_graph_edge_at(r2, p_node);
    }
}

bool if_in_neighbors(p_graph_node p_g_node, p_graph_node p_n_node) {
    p_list_head p_node;
    list_for_each(p_node, &p_g_node->neighbors) {
        p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
        if (p_neighbor == p_n_node)
            return true;
    }
    return false;
}

void node_neighbor_del(p_graph_node p_g_node, p_graph_node p_del_node) {
    p_list_head p_node;
    list_for_each(p_node, &p_g_node->neighbors) {
        p_neighbor_node p_n_node = list_entry(p_node, neighbor_node, node);
        if (p_n_node->p_neighbor == p_del_node) {
            list_del(&p_n_node->node);
            free(p_n_node);
            break;
        }
    }
}

static inline void print_conflict_node(p_graph_node p_g_node) {
    printf("%ld: {", p_g_node->node_id);
    p_list_head p_node;
    list_for_each(p_node, &p_g_node->neighbors) {
        p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
        if (p_node->p_next != &p_g_node->neighbors)
            printf("%ld, ", p_neighbor->node_id);
        else
            printf("%ld ", p_neighbor->node_id);
    }
    printf("}\n");
}
void print_conflict_graph(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        p_graph_node p_g_node = (p_graph->p_nodes + i)->p_def_node;
        print_conflict_node(p_g_node);
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->use_spill_list) {
            p_g_node = list_entry(p_node, spill_node, node)->p_spill;
            print_conflict_node(p_g_node);
        }
    }
}

void graph_node_drop(p_graph_node p_node) {
    while (!list_head_alone(&p_node->neighbors)) {
        p_neighbor_node p_neighbor = list_entry(p_node->neighbors.p_next, neighbor_node, node);
        list_del(&p_neighbor->node);
        free(p_neighbor);
    }
    free(p_node->used_color);
    free(p_node);
}

void conflict_graph_drop(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        graph_node_drop((p_graph->p_nodes + i)->p_def_node);
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &(p_graph->p_nodes + i)->use_spill_list) {
            p_spill_node p_s_node = list_entry(p_node, spill_node, node);
            graph_node_drop(p_s_node->p_spill);
            free(p_s_node);
        }
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
        list_for_each(p_node, &(p_graph->p_nodes + i)->use_spill_list) {
            p_g_node = list_entry(p_node, spill_node, node)->p_spill;
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
        p_current_node_info->visited = true;
        list_del(&p_current_node_info->node);
        p_list_head p_node;
        list_for_each(p_node, &p_current_node_info->p_node->neighbors) {
            p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
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
        list_for_each(p_node, &p_graph->seo_seq[i]->neighbors) {
            p_graph_node p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor;
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
    list_for_each(p_neighbor_node, &p_node->neighbors) {
        p_graph_node p_neighbor = list_entry(p_neighbor_node, neighbor_node, node)->p_neighbor;
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

