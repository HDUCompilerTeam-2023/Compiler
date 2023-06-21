#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>

#include <ir/vreg.h>

#include <stdio.h>

void graph_node_gen(p_graph_node p_node, p_ir_vreg p_vreg) {
    p_node->p_vreg = p_vreg;
    p_node->color = -1;
    p_node->neighbors = list_head_init(&p_node->neighbors);
}

p_neighbor_node graph_neighbor_node_gen(p_graph_node p_node){
    p_neighbor_node p_neighbor = malloc(sizeof(*p_neighbor));
    p_neighbor->node = list_head_init(&p_neighbor->node);
    p_neighbor->p_neighbor = p_node;
    return p_neighbor;
}

p_conflict_graph conflict_graph_gen(size_t node_num, size_t *map, p_graph_node p_nodes, size_t reg_num) {
    p_conflict_graph p_graph = malloc(sizeof(*p_graph));
    p_graph->node_num = node_num;
    p_graph->map = map;
    p_graph->reg_num = reg_num;
    p_graph->p_nodes = p_nodes;
    p_graph->seo_seq = malloc(sizeof(*p_graph->seo_seq) * p_graph->node_num);
    return p_graph;
}

void add_graph_edge(p_graph_node r1, p_graph_node r2) {
    p_list_head p_head = &r2->neighbors;
    p_list_head p_node = p_head;
    p_neighbor_node p_pos;
    list_for_each(p_node, p_head) {
        p_pos = list_entry(p_node, neighbor_node, node);
        if (p_pos->p_neighbor->p_vreg->id >= r1->p_vreg->id)
            break;
    }
    if (p_node == p_head || p_pos->p_neighbor->p_vreg->id > r1->p_vreg->id) {
        p_neighbor_node p_neighbor = graph_neighbor_node_gen(r1);
        list_add_prev(&p_neighbor->node, p_node);
    }

    p_head = &r1->neighbors;
    list_for_each(p_node, p_head) {
        p_pos = list_entry(p_node, neighbor_node, node);
        if (p_pos->p_neighbor->p_vreg->id >= r2->p_vreg->id)
            break;
    }
    if (p_node == p_head || p_pos->p_neighbor->p_vreg->id > r2->p_vreg->id) {
        p_neighbor_node p_neighbor = graph_neighbor_node_gen(r2);
        list_add_prev(&p_neighbor->node, p_node);
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

void print_conflict_graph(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->node_num; i++) {
        printf("%ld: {", i);
        p_list_head p_node;
        list_for_each(p_node, &(p_graph->p_nodes + i)->neighbors) {
            p_ir_vreg p_neighbor = list_entry(p_node, neighbor_node, node)->p_neighbor->p_vreg;
            if (p_node->p_next != &(p_graph->p_nodes + i)->neighbors)
                printf("%ld, ", p_neighbor->id);
            else
                printf("%ld ", p_neighbor->id);
        }
        printf("}\n");
    }
}

void conflict_graph_drop(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->node_num; i++) {
        while (!list_head_alone(&(p_graph->p_nodes + i)->neighbors)) {
            p_neighbor_node p_neighbor = list_entry((p_graph->p_nodes + i)->neighbors.p_next, neighbor_node, node);
            list_del(&p_neighbor->node);
            free(p_neighbor);
        }
    }
    free(p_graph->p_nodes);
    free(p_graph->seo_seq);
    free(p_graph->map);
    free(p_graph);
}