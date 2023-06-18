#include <ir_gen.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <program/def.h>
#include <symbol_gen.h>

static inline p_ir_basic_block cut_edge(p_ir_basic_block p_basic_block, p_ir_basic_block_branch_target p_target, p_symbol_func p_func) {
    p_ir_basic_block p_new_basic_block = ir_basic_block_gen();
    list_add_next(&p_new_basic_block->node, &p_basic_block->node);
    ir_basic_block_add_prev(p_basic_block, p_new_basic_block);
    p_func->block_cnt ++;
    p_new_basic_block->p_dom_parent = p_basic_block;
    p_new_basic_block->p_branch->kind = ir_br_branch;
    p_new_basic_block->p_branch->p_target_1 = p_target;

    p_list_head p_node;
    list_for_each(p_node, &p_new_basic_block->p_branch->p_target_1->p_block->prev_basic_block_list) {
        p_ir_basic_block_list_node p_prev = list_entry(p_node, ir_basic_block_list_node, node);
        if (p_prev->p_basic_block == p_basic_block)
            p_prev->p_basic_block = p_new_basic_block;
    }
    return p_new_basic_block;
}

void critical_edge_cut_pass(p_program p_ir) {
    p_list_head p_func_node;
    list_for_each(p_func_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_func_node, symbol_func, node);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
            if (p_basic_block->p_branch->kind == ir_cond_branch) {
                if (!list_head_alone(&p_basic_block->p_branch->p_target_1->p_block_param->bb_param)) {
                    // 新建块处理target2 的 block_param
                    p_ir_basic_block p_new_basic_block = cut_edge(p_basic_block, p_basic_block->p_branch->p_target_1, p_func);
                    p_basic_block->p_branch->p_target_1 = ir_basic_block_branch_target_gen(p_new_basic_block);
                }
                if (!list_head_alone(&p_basic_block->p_branch->p_target_2->p_block_param->bb_param)) {
                    // 新建块处理target2 的 block_param
                    p_ir_basic_block p_new_basic_block = cut_edge(p_basic_block, p_basic_block->p_branch->p_target_2, p_func);
                    p_basic_block->p_branch->p_target_2 = ir_basic_block_branch_target_gen(p_new_basic_block);
                }
            }
        }
        symbol_func_set_block_id(p_func);
    }
}