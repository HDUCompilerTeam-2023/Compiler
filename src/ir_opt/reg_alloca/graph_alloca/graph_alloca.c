#include <ir_opt/reg_alloca/graph_alloca/liveness.h>

#include <ir_gen.h>
#include <symbol_gen.h>
// 生成浮点和通用寄存器的节点和映射，并生成对应的图
// 初始化所有指令和基本块的活跃变量集合
p_graph_alloca_info graph_alloca_info_gen(size_t reg_num_r, size_t reg_num_s, p_symbol_func p_func) {
    p_graph_alloca_info p_info = malloc(sizeof(*p_info));
    p_info->p_func = p_func;
    p_list_head p_node;

    size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;

    p_origin_graph_node p_nodes_r = malloc(sizeof(*p_nodes_r) * vreg_num);
    p_origin_graph_node p_nodes_s = malloc(sizeof(*p_nodes_s) * vreg_num);
    size_t numr = 0;
    size_t nums = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            origin_graph_node_gen(p_nodes_s + nums, p_vreg, reg_num_s, nums);
            nums++;
        }
        else {
            origin_graph_node_gen(p_nodes_r + numr, p_vreg, reg_num_r, numr);
            numr++;
        }
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            origin_graph_node_gen(p_nodes_s + nums, p_vreg, reg_num_s, nums);
            nums++;
        }
        else {
            origin_graph_node_gen(p_nodes_r + numr, p_vreg, reg_num_r, numr);
            numr++;
        }
    }

    p_info->p_r_graph = conflict_graph_gen(numr, p_nodes_r, reg_num_r);
    p_info->p_s_graph = conflict_graph_gen(nums, p_nodes_s, reg_num_s);
    p_info->block_live_in = malloc(sizeof(void *) * p_func->block_cnt);
    p_info->block_live_out = malloc(sizeof(void *) * p_func->block_cnt);
    p_info->block_branch_live_in = malloc(sizeof(void *) * p_func->block_cnt);
    for (size_t i = 0; i < p_func->block_cnt; i++) {
        p_info->block_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_in[i]);
        p_info->block_live_out[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_live_out[i]);
        p_info->block_branch_live_in[i] = bitmap_gen(vreg_num);
        bitmap_set_empty(p_info->block_branch_live_in[i]);
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
        bitmap_drop(p_info->block_branch_live_in[i]);
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
    free(p_info->block_branch_live_in);
    free(p_info->instr_live_in);
    free(p_info->instr_live_out);
    free(p_info->p_vregs);
    free(p_info);
}

// 对传入的参数进行预着色
void pre_color(p_graph_alloca_info p_info, p_symbol_func p_func) {
    p_list_head p_node;
    size_t current_r = 0;
    size_t current_s = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_graph_node p_g_node = (p_graph_node) p_vreg->p_info;
        if (p_vreg->if_float) {
            set_node_color(p_info->p_s_graph, p_g_node, current_s);
            current_s++;
        }
        else {
            set_node_color(p_info->p_r_graph, p_g_node, current_r);
            current_r++;
        }
    }
}

static void new_store(p_conflict_graph p_graph, p_ir_vreg p_vreg, p_ir_instr p_instr, p_ir_bb_phi_list p_live_out, p_symbol_func p_func) {
    if (((p_graph_node) p_vreg->p_info)->node_id >= p_graph->origin_node_num) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + ((p_graph_node) p_vreg->p_info)->node_id;
    if (!p_o_node->if_need_spill) return;
    assert(!p_o_node->p_vmem);
    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
    symbol_func_add_variable(p_func, p_vmem);
    p_o_node->p_vmem = p_vmem;
    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem), NULL, ir_operand_vreg_gen(p_vreg));
    list_add_next(&p_store->node, &p_instr->node);
    // 改变活跃集合
    copy_live(p_store->p_live_out, p_live_out);
    ir_bb_phi_list_add(p_live_out, p_vreg);
    copy_live(p_store->p_live_in, p_live_out);
    // 改变干涉图, 活跃集合和邻居节点都是从小到大编号
    p_list_head p_live_node = p_store->p_live_out->bb_phi.p_next;
    p_list_head p_neigh_node = p_o_node->p_def_node->neighbors.p_next;
    while (p_live_node != &p_store->p_live_out->bb_phi) {
        p_ir_vreg p_live_vreg = list_entry(p_live_node, ir_bb_phi, node)->p_bb_phi;
        p_neighbor_node p_neighbor = list_entry(p_neigh_node, neighbor_node, node);
        if (p_live_vreg == p_neighbor->p_neighbor->p_vreg) {
            p_live_node = p_live_node->p_next;
            p_neigh_node = p_neigh_node->p_next;
            continue;
        }
        assert(p_live_vreg->id > p_neighbor->p_neighbor->p_vreg->id); // 若定义的变量出口活跃,活跃集必是邻居子集
        node_neighbor_del(p_neighbor->p_neighbor, p_o_node->p_def_node);
        p_neigh_node = p_neigh_node->p_next;
        list_del(&p_neighbor->node);
        free(p_neighbor);
    }
    while (p_neigh_node != &p_o_node->p_def_node->neighbors) {
        p_neighbor_node p_neighbor = list_entry(p_neigh_node, neighbor_node, node);
        node_neighbor_del(p_neighbor->p_neighbor, p_o_node->p_def_node);
        p_neigh_node = p_neigh_node->p_next;
        list_del(&p_neighbor->node);
        free(p_neighbor);
    }
}

static void new_load(p_conflict_graph p_graph, p_ir_operand p_operand, p_ir_instr p_instr, p_ir_bb_phi_list p_live_in, p_symbol_func p_func) {
    if (p_operand->kind != reg) return;
    p_graph_node p_vreg_g_node = (p_graph_node) p_operand->p_vreg->p_info;
    if (p_vreg_g_node->node_id >= p_graph->origin_node_num) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_vreg_g_node->node_id;
    if (!p_o_node->if_need_spill) return;
    assert(p_o_node->p_vmem);
    p_ir_vreg p_new_src = ir_vreg_copy(p_operand->p_vreg);
    symbol_func_vreg_add(p_func, p_new_src);
    p_graph_node p_g_node = graph_node_gen(p_new_src, p_graph->reg_num, p_graph->node_num);
    p_graph->node_num++;
    spill_list_add(p_o_node, p_g_node);
    p_symbol_var p_vmem = p_o_node->p_vmem;
    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem), NULL, p_new_src);
    list_add_prev(&p_load->node, &p_instr->node);
    p_operand->p_vreg = p_new_src;
    // 改变活跃集合和冲突图
    copy_live(p_load->p_live_in, p_live_in);
    ir_bb_phi_list_add(p_live_in, p_new_src);
    copy_live(p_load->p_live_out, p_live_in);
    p_list_head p_node;
    list_for_each(p_node, &p_load->p_live_in->bb_phi) {
        p_ir_vreg p_live_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        add_graph_edge(p_g_node, (p_graph_node) p_live_vreg->p_info);
    }
}

static inline void deal_live_set(p_conflict_graph p_graph, p_ir_bb_phi_list p_live) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_live->bb_phi) {
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        p_graph_node p_g_node = (p_graph_node) p_phi->p_bb_phi->p_info;
        if (p_g_node->node_id >= p_graph->origin_node_num) continue;
        if ((p_graph->p_nodes + p_g_node->node_id)->if_need_spill) {
            list_del(p_node);
            free(p_phi);
        }
    }
}

void graph_spill(p_conflict_graph p_graph, p_symbol_func p_func) {
    p_list_head p_instr_node = NULL;
    p_list_head p_node;
    // 处理形参
    p_ir_basic_block p_entry = list_entry(p_func->block.p_next, ir_basic_block, node);
    p_instr_node = p_entry->instr_list.p_next;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
        new_store(p_graph, p_param, list_entry(&p_entry->instr_list, ir_instr, node), p_entry->p_live_in, p_func);
    }
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        deal_live_set(p_graph, p_basic_block->p_live_in);
        deal_live_set(p_graph, p_basic_block->p_live_out);
        p_ir_instr p_head_instr = list_entry(&p_basic_block->instr_list, ir_instr, node);
        if (!p_instr_node)
            p_instr_node = p_basic_block->instr_list.p_next;
        list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
            p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            new_store(p_graph, p_phi, p_head_instr, p_basic_block->p_live_in, p_func);
        }

        p_list_head p_instr_node_next;
        while (p_instr_node != &p_basic_block->instr_list) {
            p_instr_node_next = p_instr_node->p_next;
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_live_set(p_graph, p_instr->p_live_in);
            deal_live_set(p_graph, p_instr->p_live_out);
            switch (p_instr->irkind) {
            case ir_binary:
                new_load(p_graph, p_instr->ir_binary.p_src1, p_instr, p_instr->p_live_in, p_func);
                new_load(p_graph, p_instr->ir_binary.p_src2, p_instr, p_instr->p_live_in, p_func);
                new_store(p_graph, p_instr->ir_binary.p_des, p_instr, p_instr->p_live_out, p_func);
                break;
            case ir_unary:
                new_load(p_graph, p_instr->ir_unary.p_src, p_instr, p_instr->p_live_in, p_func);
                new_store(p_graph, p_instr->ir_unary.p_des, p_instr, p_instr->p_live_out, p_func);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    new_load(p_graph, p_param, p_instr, p_instr->p_live_in, p_func);
                }
                if (p_instr->ir_call.p_des)
                    new_store(p_graph, p_instr->ir_call.p_des, p_instr, p_instr->p_live_out, p_func);
                break;
            case ir_load:
                new_load(p_graph, p_instr->ir_load.p_addr, p_instr, p_instr->p_live_in, p_func);
                if (p_instr->ir_load.p_offset)
                    new_load(p_graph, p_instr->ir_load.p_offset, p_instr, p_instr->p_live_in, p_func);
                new_store(p_graph, p_instr->ir_load.p_des, p_instr, p_instr->p_live_out, p_func);
                break;
            case ir_store:
                new_load(p_graph, p_instr->ir_store.p_src, p_instr, p_instr->p_live_in, p_func);
                new_load(p_graph, p_instr->ir_store.p_addr, p_instr, p_instr->p_live_in, p_func);
                if (p_instr->ir_store.p_offset)
                    new_load(p_graph, p_instr->ir_store.p_offset, p_instr, p_instr->p_live_in, p_func);
                break;
            case ir_gep:
                assert(0);
            }
            p_instr_node = p_instr_node_next;
        }

        deal_live_set(p_graph, p_basic_block->p_branch->p_live_in);
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_graph, p_param, p_head_instr, p_basic_block->p_live_out, p_func);
            }
            break;
        case ir_cond_branch:
            new_load(p_graph, p_basic_block->p_branch->p_exp, p_head_instr, p_basic_block->p_live_out, p_func);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_graph, p_param, p_head_instr, p_basic_block->p_live_out, p_func);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_graph, p_param, p_head_instr, p_basic_block->p_live_out, p_func);
            }
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                new_load(p_graph, p_basic_block->p_branch->p_exp, p_head_instr, p_basic_block->p_live_out, p_func);
            break;
        case ir_abort_branch:
            assert(0);
        }
        p_instr_node = NULL;
    }
}

void graph_alloca(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s) {
    p_graph_alloca_info p_info = graph_alloca_info_gen(reg_num_r, reg_num_s, p_func);
    liveness_analysis(p_info, p_func);
    set_func_live(p_info, p_func);

    graph_nodes_init(p_info->p_r_graph);
    graph_nodes_init(p_info->p_s_graph);
    pre_color(p_info, p_func);

    print_conflict_graph(p_info->p_r_graph);
    mcs_get_seqs(p_info->p_r_graph);
    set_graph_color(p_info->p_r_graph);
    check_chordal(p_info->p_r_graph);
    print_conflict_graph(p_info->p_s_graph);
    mcs_get_seqs(p_info->p_s_graph);
    set_graph_color(p_info->p_s_graph);
    check_chordal(p_info->p_s_graph);

    check_liveness(p_func);
    graph_alloca_info_drop(p_info);
}