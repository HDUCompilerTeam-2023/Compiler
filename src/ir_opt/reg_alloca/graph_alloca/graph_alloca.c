#include <ir_opt/reg_alloca/graph_alloca/liveness.h>

#include <symbol/func.h>
#include <ir/basic_block.h>
#include <ir/instr.h>
#include <ir/vreg.h>
// 生成浮点和通用寄存器的节点和映射，并生成对应的图
// 初始化所有指令和基本块的活跃变量集合
p_graph_alloca_info graph_alloca_info_gen(size_t reg_num_r, size_t reg_num_s, p_symbol_func p_func) {
    p_graph_alloca_info p_info = malloc(sizeof(*p_info));
    p_info->p_func = p_func;
    p_list_head p_node;
    size_t *mapr = malloc(sizeof(*mapr) * (p_func->param_reg_cnt + p_func->vreg_cnt));
    size_t *maps = malloc(sizeof(*maps) * (p_func->param_reg_cnt + p_func->vreg_cnt));
    size_t numr = 0;
    size_t nums = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float)
            maps[p_vreg->id] = nums++;
        else
            mapr[p_vreg->id] = numr++;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float)
            maps[p_vreg->id] = nums++;
        else
            mapr[p_vreg->id] = numr++;
    }

    p_graph_node p_nodes_r = malloc(sizeof(*p_nodes_r) * numr);
    p_graph_node p_nodes_s = malloc(sizeof(*p_nodes_s) * nums);
    numr = nums = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            graph_node_gen(p_nodes_s + nums, p_vreg);
            nums++;
        }
        else {
            graph_node_gen(p_nodes_r + numr, p_vreg);
            numr++;
        }
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            graph_node_gen(p_nodes_s + nums, p_vreg);
            nums++;
        }
        else {
            graph_node_gen(p_nodes_r + numr, p_vreg);
            numr++;
        }
    }

    p_info->p_r_graph = conflict_graph_gen(numr, mapr, p_nodes_r, reg_num_r);
    p_info->p_s_graph = conflict_graph_gen(nums, maps, p_nodes_s, reg_num_s);
    size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_info->block_live_in = malloc(sizeof(void *) * p_func->block_cnt);
    p_info->block_live_out = malloc(sizeof(void *) * p_func->block_cnt);
    for (size_t i = 0; i < p_func->block_cnt; i++) {
        p_info->block_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_in[i]);
        *(p_info->block_live_out + i) = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_out[i]);
    }
    p_ir_basic_block p_last_block = list_entry(p_func->block.p_prev, ir_basic_block, node);
    while (list_head_alone(&p_last_block->instr_list))
        p_last_block = list_entry(p_last_block->node.p_prev, ir_basic_block, node);
    size_t instr_num = list_entry(p_last_block->instr_list.p_prev, ir_instr, node)->instr_id + 1;
    p_info->instr_live_in = malloc(sizeof(void *) * instr_num);
    p_info->instr_live_out = malloc(sizeof(void *) * instr_num);
    for (size_t i = 0; i < instr_num; i++) {
        p_info->instr_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_in[i]);
        p_info->instr_live_out[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->instr_live_out[i]);
    }
    p_info->p_vregs = malloc(sizeof(void *) * vreg_num);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_info->p_vregs[p_vreg->id] = p_vreg;
    }
    return p_info;
}

void graph_alloca_info_drop(p_graph_alloca_info p_info) {
    conflict_graph_drop(p_info->p_r_graph);
    conflict_graph_drop(p_info->p_s_graph);
    for (size_t i = 0; i < p_info->p_func->block_cnt; i++) {
        bitmap_drop(p_info->block_live_in[i]);
        bitmap_drop(p_info->block_live_out[i]);
    }
    p_ir_basic_block p_last_block = list_entry(p_info->p_func->block.p_prev, ir_basic_block, node);
    while (list_head_alone(&p_last_block->instr_list))
        p_last_block = list_entry(p_last_block->node.p_prev, ir_basic_block, node);
    size_t instr_num = list_entry(p_last_block->instr_list.p_prev, ir_instr, node)->instr_id + 1;
    for (size_t i = 0; i < instr_num; i++) {
        bitmap_drop(p_info->instr_live_in[i]);
        bitmap_drop(p_info->instr_live_out[i]);
    }
    free(p_info->block_live_in);
    free(p_info->block_live_out);
    free(p_info->instr_live_in);
    free(p_info->instr_live_out);
    free(p_info->p_vregs);
    free(p_info);
}

void graph_alloca(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s) {
    p_graph_alloca_info p_info = graph_alloca_info_gen(reg_num_r, reg_num_s, p_func);
    liveness_analysis(p_info, p_func);
    print_conflict_graph(p_info->p_r_graph);
    print_conflict_graph(p_info->p_s_graph);
    set_func_live(p_info, p_func);
    check_liveness(p_func);
    graph_alloca_info_drop(p_info);
}