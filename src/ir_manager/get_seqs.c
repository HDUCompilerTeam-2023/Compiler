#include <ir_gen.h>
#include <ir_manager/buildnestree.h>
#include <symbol_gen.h>
typedef struct in_degree_node in_degree_node, *p_in_degree_node;
struct in_degree_node {
    size_t in_degree;
    p_ir_basic_block p_block;
    list_head node;
};
static inline p_in_degree_node in_degree_node_gen(p_ir_basic_block p_block) {
    p_in_degree_node p_in_node = malloc(sizeof(*p_in_node));
    p_in_node->in_degree = p_block->prev_num;
    p_in_node->p_block = p_block;
    p_in_node->node = list_head_init(&p_in_node->node);
    p_block->p_info = p_in_node;
    return p_in_node;
}
static inline void in_degree_node_drop(p_in_degree_node p_node) {
    list_del(&p_node->node);
    free(p_node);
}
p_list_head get_topo_seqs(p_symbol_func p_func) {
    p_list_head p_head = malloc(sizeof(*p_head));
    list_head_init(p_head);
    if (list_head_alone(&p_func->block)) return p_head;
    list_head *in_degree_blocks = malloc(sizeof(*in_degree_blocks) * p_func->block_cnt);
    for (size_t i = 0; i < p_func->block_cnt; i++) {
        in_degree_blocks[i] = list_head_init(in_degree_blocks + i);
    }
    symbol_func_basic_block_init_visited(p_func);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_block = list_entry(p_node, ir_basic_block, node);
        p_block->seqs_node = list_head_init(&p_block->seqs_node);
        p_in_degree_node p_block_node = in_degree_node_gen(p_block);
        list_add_prev(&p_block_node->node, in_degree_blocks + p_block->prev_num);
    }
    while (!list_head_alone(in_degree_blocks + 0)) {
        p_in_degree_node p_block_node = list_entry((in_degree_blocks + 0)->p_next, in_degree_node, node);
        p_ir_basic_block p_block = p_block_node->p_block;
        p_block->if_visited = true;
        in_degree_node_drop(p_block_node);
        list_add_prev(&p_block->seqs_node, p_head);
        p_ir_basic_block_branch_target p_target1 = p_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_target2 = p_block->p_branch->p_target_2;
        if (p_target1 && !p_target1->p_block->if_visited) {
            p_in_degree_node p_target_block_node = p_target1->p_block->p_info;
            assert(p_target_block_node->in_degree);
            list_del(&p_target_block_node->node);
            p_target_block_node->in_degree--;
            list_add_prev(&p_target_block_node->node, in_degree_blocks + p_target_block_node->in_degree);
        }
        if (p_target2 && !p_target2->p_block->if_visited) {
            p_in_degree_node p_target_block_node = p_target2->p_block->p_info;
            assert(p_target_block_node->in_degree);
            list_del(&p_target_block_node->node);
            p_target_block_node->in_degree--;
            list_add_prev(&p_target_block_node->node, in_degree_blocks + p_target_block_node->in_degree);
        }
    }
    for (size_t i = 0; i < p_func->block_cnt; i++) {
        assert(list_head_alone(in_degree_blocks + i));
    }
    free(in_degree_blocks);
    return p_head;
}

static inline void get_rpo_seqs_block(p_ir_basic_block p_block, p_list_head p_head) {
    if (p_block->if_visited) return;
    p_block->if_visited = true;
    p_ir_basic_block_branch_target p_target1 = p_block->p_branch->p_target_1;
    p_ir_basic_block_branch_target p_target2 = p_block->p_branch->p_target_2;
    if (p_target1)
        get_rpo_seqs_block(p_target1->p_block, p_head);
    if (p_target2)
        get_rpo_seqs_block(p_target2->p_block, p_head);
    p_block->seqs_node = list_head_init(&p_block->seqs_node);
    list_add_next(&p_block->seqs_node, p_head);
}
p_list_head get_rpo_seqs(p_symbol_func p_func) {
    p_list_head p_head = malloc(sizeof(*p_head));
    list_head_init(p_head);
    if (list_head_alone(&p_func->block)) return p_head;
    symbol_func_basic_block_init_visited(p_func);
    get_rpo_seqs_block(p_func->p_entry_block, p_head);
    assert(!list_head_alone(p_head));
    return p_head;
}