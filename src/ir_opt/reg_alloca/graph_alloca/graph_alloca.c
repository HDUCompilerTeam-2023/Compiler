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
    p_info->p_r_graph = conflict_graph_gen(reg_num_r, p_func);
    p_info->p_s_graph = conflict_graph_gen(reg_num_s, p_func);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            origin_graph_node_gen(p_nodes_s + nums, p_vreg, p_info->p_s_graph);
            nums++;
        }
        else {
            origin_graph_node_gen(p_nodes_r + numr, p_vreg, p_info->p_r_graph);
            numr++;
        }
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float) {
            origin_graph_node_gen(p_nodes_s + nums, p_vreg, p_info->p_s_graph);
            nums++;
        }
        else {
            origin_graph_node_gen(p_nodes_r + numr, p_vreg, p_info->p_r_graph);
            numr++;
        }
    }
    conflict_graph_set_nodes(p_info->p_r_graph, p_nodes_r, numr);
    conflict_graph_set_nodes(p_info->p_s_graph, p_nodes_s, nums);
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
    p_info->instr_num = instr_num;
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

    for (size_t i = 0; i < p_info->instr_num; i++) {
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

static inline p_conflict_graph get_graph(p_graph_alloca_info p_info, p_ir_vreg p_vreg) {
    if (p_vreg->if_float)
        return p_info->p_s_graph;
    return p_info->p_r_graph;
}

static inline void update_graph(p_graph_alloca_info p_info, p_ir_bb_phi_list p_store_live, p_graph_node p_g_node) {
    // 改变干涉图, 活跃集合和邻居节点都是从小到大编号
    p_list_head p_live_node = p_store_live->bb_phi.p_next;
    p_list_head p_neigh_node = p_g_node->p_neighbors->node.p_next;
    while (p_live_node != &p_store_live->bb_phi) {
        p_ir_vreg p_live_vreg = list_entry(p_live_node, ir_bb_phi, node)->p_bb_phi;
        if (p_live_vreg->if_float != p_g_node->p_vreg->if_float) {
            p_live_node = p_live_node->p_next;
            continue;
        }
        p_graph_nodes p_neighbor = list_entry(p_neigh_node, graph_nodes, node);
        if (p_live_vreg == p_neighbor->p_node->p_vreg) {
            p_live_node = p_live_node->p_next;
            p_neigh_node = p_neigh_node->p_next;
            continue;
        }
        assert(p_live_vreg->id > p_neighbor->p_node->p_vreg->id); // 若定义的变量出口活跃,活跃集必是邻居子集
        node_list_del(p_neighbor->p_node->p_neighbors, p_g_node);
        p_neigh_node = p_neigh_node->p_next;
        list_del(&p_neighbor->node);
        free(p_neighbor);
    }
    while (p_neigh_node != &p_g_node->p_neighbors->node) {
        p_graph_nodes p_neighbor = list_entry(p_neigh_node, graph_nodes, node);
        node_list_del(p_neighbor->p_node->p_neighbors, p_g_node);
        p_neigh_node = p_neigh_node->p_next;
        list_del(&p_neighbor->node);
        free(p_neighbor);
    }
}

static inline bool check_spill(p_conflict_graph p_graph, p_graph_node p_g_node, p_symbol_func p_func) {
    if (p_g_node->node_id >= p_graph->origin_node_num) return false;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;
    if (!p_o_node->if_need_spill) return false;
    assert(p_o_node->p_vmem);
    return true;
}

static inline void live_list_add(p_ir_bb_phi_list p_live, p_ir_vreg p_vreg) {
    p_list_head p_head = p_live->bb_phi.p_next;
    p_list_head p_tail = p_live->bb_phi.p_prev;
    while (p_head != &p_live->bb_phi) {
        p_ir_vreg p_live_vreg_head = list_entry(p_head, ir_bb_phi, node)->p_bb_phi;
        p_ir_vreg p_live_vreg_tail = list_entry(p_tail, ir_bb_phi, node)->p_bb_phi;
        if (p_live_vreg_head == p_vreg) return;
        if (p_live_vreg_tail == p_vreg) return;
        if (p_live_vreg_head->id >= p_vreg->id) {
            p_ir_bb_phi p_live_vreg = malloc(sizeof(*p_live_vreg));
            p_live_vreg->node = list_head_init(&p_live_vreg->node);
            p_live_vreg->p_bb_phi = p_vreg;
            list_add_prev(&p_live_vreg->node, p_head);
            return;
        }
        if (p_live_vreg_tail->id <= p_vreg->id) {
            p_ir_bb_phi p_live_vreg = malloc(sizeof(*p_live_vreg));
            p_live_vreg->node = list_head_init(&p_live_vreg->node);
            p_live_vreg->p_bb_phi = p_vreg;
            list_add_next(&p_live_vreg->node, p_tail);
            return;
        }
        p_head = p_head->p_next;
        p_tail = p_tail->p_prev;
    }
    // 活跃集为空
    p_ir_bb_phi p_live_vreg = malloc(sizeof(*p_live_vreg));
    p_live_vreg->node = list_head_init(&p_live_vreg->node);
    p_live_vreg->p_bb_phi = p_vreg;
    list_add_next(&p_live_vreg->node, p_head);
}

static void new_store_front(p_graph_alloca_info p_info, p_ir_vreg p_vreg, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_conflict_graph p_graph = get_graph(p_info, p_vreg);
    p_graph_node p_g_node = (p_graph_node) p_vreg->p_info;
    if (!check_spill(p_graph, p_g_node, p_func)) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;
    p_ir_bb_phi_list p_live_in;
    if (list_head_alone(&p_basic_block->instr_list))
        p_live_in = p_basic_block->p_branch->p_live_in;
    else {
        p_ir_instr p_first_instr = list_entry(p_basic_block->instr_list.p_next, ir_instr, node);
        p_live_in = p_first_instr->p_live_in;
    }
    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_o_node->p_vmem), NULL, ir_operand_vreg_gen(p_vreg));
    list_add_next(&p_store->node, &p_basic_block->instr_list);
    copy_live(p_store->p_live_out, p_live_in);
    copy_live(p_store->p_live_in, p_live_in);
    live_list_add(p_store->p_live_in, p_vreg);
}

static void new_store_middle(p_graph_alloca_info p_info, p_ir_vreg p_vreg, p_ir_instr p_instr, p_symbol_func p_func) {
    p_graph_node p_g_node = (p_graph_node) p_vreg->p_info;
    p_conflict_graph p_graph = get_graph(p_info, p_vreg);
    if (!check_spill(p_graph, p_g_node, p_func)) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;
    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_o_node->p_vmem), NULL, ir_operand_vreg_gen(p_vreg));
    list_add_next(&p_store->node, &p_instr->node);
    // 改变活跃集合
    copy_live(p_store->p_live_out, p_instr->p_live_out);
    live_list_add(p_instr->p_live_out, p_vreg);
    copy_live(p_store->p_live_in, p_instr->p_live_out);
    update_graph(p_info, p_store->p_live_out, p_o_node->p_def_node);
}

static void new_load(p_graph_alloca_info p_info, p_ir_operand p_operand, p_ir_instr p_instr, p_ir_bb_phi_list p_live_in, p_symbol_func p_func) {
    if (p_operand->kind != reg) return;
    p_graph_node p_vreg_g_node = (p_graph_node) p_operand->p_vreg->p_info;
    p_conflict_graph p_graph = get_graph(p_info, p_operand->p_vreg);
    if (!check_spill(p_graph, p_vreg_g_node, p_func)) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_vreg_g_node->node_id;
    p_ir_vreg p_new_src = ir_vreg_copy(p_operand->p_vreg);
    symbol_func_vreg_add(p_func, p_new_src);
    p_graph_node p_g_node = graph_node_gen(p_new_src, p_graph);
    graph_node_list_add(p_o_node->p_use_spill_list, p_g_node);
    p_symbol_var p_vmem = p_o_node->p_vmem;
    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem), NULL, p_new_src);
    list_add_prev(&p_load->node, &p_instr->node);
    p_operand->p_vreg = p_new_src;
    // 改变活跃集合和冲突图
    copy_live(p_load->p_live_in, p_live_in);
    live_list_add(p_live_in, p_new_src);
    copy_live(p_load->p_live_out, p_live_in);
    p_list_head p_node;
    list_for_each(p_node, &p_load->p_live_in->bb_phi) {
        p_ir_vreg p_live_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        add_reg_graph_edge(p_new_src, p_live_vreg);
    }
}

static inline void deal_live_set(p_graph_alloca_info p_info, p_ir_bb_phi_list p_live) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_live->bb_phi) {
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        p_conflict_graph p_graph = get_graph(p_info, p_phi->p_bb_phi);
        p_graph_node p_g_node = (p_graph_node) p_phi->p_bb_phi->p_info;
        if (p_g_node->node_id >= p_graph->origin_node_num) continue;
        if ((p_graph->p_nodes + p_g_node->node_id)->if_need_spill) {
            list_del(p_node);
            free(p_phi);
        }
    }
}

static inline void new_store_bb_phi_front(p_graph_alloca_info p_info, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_list_head p_node;
    p_list_head p_first_node = p_basic_block->instr_list.p_next;
    list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        new_store_front(p_info, p_phi, p_basic_block, p_func);
    }
    p_node = p_basic_block->instr_list.p_next;
    while (p_node != p_first_node) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        update_graph(p_info, p_instr->p_live_out, (p_graph_node) p_instr->ir_store.p_src->p_vreg->p_info);
        p_node = p_node->p_next;
    }
}
static inline void new_store_param_front(p_graph_alloca_info p_info, p_symbol_func p_func) {
    // 处理形参
    p_ir_basic_block p_entry = list_entry(p_func->block.p_next, ir_basic_block, node);
    p_list_head p_node;
    p_list_head p_first_node = p_entry->instr_list.p_next;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        new_store_front(p_info, p_vreg, p_entry, p_func);
    }
    p_node = p_entry->instr_list.p_next;
    while (p_node != p_first_node) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        update_graph(p_info, p_instr->p_live_out, (p_graph_node) p_instr->ir_store.p_src->p_vreg->p_info);
        p_node = p_node->p_next;
    }
}

void graph_spill(p_graph_alloca_info p_info, p_symbol_func p_func) {
    p_list_head p_node;
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        deal_live_set(p_info, p_basic_block->p_live_in);
        deal_live_set(p_info, p_basic_block->p_live_out);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_live_set(p_info, p_instr->p_live_in);
            deal_live_set(p_info, p_instr->p_live_out);
            switch (p_instr->irkind) {
            case ir_binary:
                new_load(p_info, p_instr->ir_binary.p_src1, p_instr, p_instr->p_live_in, p_func);
                new_load(p_info, p_instr->ir_binary.p_src2, p_instr, p_instr->p_live_in, p_func);
                new_store_middle(p_info, p_instr->ir_binary.p_des, p_instr, p_func);
                break;
            case ir_unary:
                new_load(p_info, p_instr->ir_unary.p_src, p_instr, p_instr->p_live_in, p_func);
                new_store_middle(p_info, p_instr->ir_unary.p_des, p_instr, p_func);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    new_load(p_info, p_param, p_instr, p_instr->p_live_in, p_func);
                }
                if (p_instr->ir_call.p_des)
                    new_store_middle(p_info, p_instr->ir_call.p_des, p_instr, p_func);
                break;
            case ir_load:
                new_load(p_info, p_instr->ir_load.p_addr, p_instr, p_instr->p_live_in, p_func);
                if (p_instr->ir_load.p_offset)
                    new_load(p_info, p_instr->ir_load.p_offset, p_instr, p_instr->p_live_in, p_func);
                new_store_middle(p_info, p_instr->ir_load.p_des, p_instr, p_func);
                break;
            case ir_store:
                new_load(p_info, p_instr->ir_store.p_src, p_instr, p_instr->p_live_in, p_func);
                new_load(p_info, p_instr->ir_store.p_addr, p_instr, p_instr->p_live_in, p_func);
                if (p_instr->ir_store.p_offset)
                    new_load(p_info, p_instr->ir_store.p_offset, p_instr, p_instr->p_live_in, p_func);
                break;
            case ir_gep:
                assert(0);
            }
        }

        deal_live_set(p_info, p_basic_block->p_branch->p_live_in);
        p_ir_instr p_head_instr = list_entry(&p_basic_block->instr_list, ir_instr, node);
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_info, p_param, p_head_instr, p_basic_block->p_branch->p_live_in, p_func);
            }
            break;
        case ir_cond_branch:
            new_load(p_info, p_basic_block->p_branch->p_exp, p_head_instr, p_basic_block->p_branch->p_live_in, p_func);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_info, p_param, p_head_instr, p_basic_block->p_branch->p_live_in, p_func);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load(p_info, p_param, p_head_instr, p_basic_block->p_branch->p_live_in, p_func);
            }
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                new_load(p_info, p_basic_block->p_branch->p_exp, p_head_instr, p_basic_block->p_branch->p_live_in, p_func);
            break;
        case ir_abort_branch:
            assert(0);
        }
        new_store_bb_phi_front(p_info, p_basic_block, p_func);
    }
    new_store_param_front(p_info, p_func);
}

static inline void combine_remap_operand(p_ir_operand p_operand, p_ir_vreg *p_map) {
    if (p_operand->kind == reg) {
        p_operand->p_vreg = p_map[p_operand->p_vreg->id];
    }
}

static inline void combine_map_self(p_ir_vreg p_vreg, p_ir_vreg *p_map) {
    p_map[p_vreg->id] = p_vreg;
}

static inline void live_set_swap(p_ir_bb_phi_list p_live, p_ir_vreg *p_map, size_t num) {
    p_list_head p_node, p_node_next;
    bool *if_have = malloc(sizeof(*if_have) * num);
    memset(if_have, false, sizeof(*if_have) * num);
    list_for_each_safe(p_node, p_node_next, &p_live->bb_phi) {
        p_ir_bb_phi p_live = list_entry(p_node, ir_bb_phi, node);
        p_ir_vreg p_new_vreg = p_map[p_live->p_bb_phi->id];
        if (if_have[p_new_vreg->id]) {
            list_del(p_node);
            free(p_live);
            continue;
        }
        if_have[p_new_vreg->id] = true;
        p_live->p_bb_phi = p_new_vreg;
    }
    free(if_have);
}

static void combine_mov(p_symbol_func p_func) {
    symbol_func_set_vreg_id(p_func);
    size_t reg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_ir_vreg *p_map = malloc(sizeof(void *) * reg_num);
    p_ir_vreg *p_need_del = malloc(sizeof(void *) * reg_num);
    size_t need_del_num = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        combine_map_self(p_vreg, p_map);
    }

    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        live_set_swap(p_basic_block->p_live_in, p_map, reg_num);
        list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
            p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            combine_map_self(p_phi, p_map);
        }

        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            live_set_swap(p_instr->p_live_in, p_map, reg_num);
            switch (p_instr->irkind) {
            case ir_binary:
                combine_remap_operand(p_instr->ir_binary.p_src1, p_map);
                combine_remap_operand(p_instr->ir_binary.p_src2, p_map);
                combine_map_self(p_instr->ir_binary.p_des, p_map);
                break;
            case ir_unary:
                combine_remap_operand(p_instr->ir_unary.p_src, p_map);
                if (p_instr->ir_unary.op == ir_val_assign && p_instr->ir_unary.p_src->kind == reg) {
                    if (p_instr->ir_unary.p_src->p_vreg->if_float == p_instr->ir_unary.p_des->if_float
                        && p_instr->ir_unary.p_src->p_vreg->reg_id == p_instr->ir_unary.p_des->reg_id) {
                        p_map[p_instr->ir_unary.p_des->id] = p_instr->ir_unary.p_src->p_vreg;
                        p_need_del[need_del_num++] = p_instr->ir_unary.p_des;
                        ir_instr_drop(p_instr);
                        continue;
                    }
                }
                combine_map_self(p_instr->ir_unary.p_des, p_map);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    combine_remap_operand(p_param, p_map);
                }
                if (p_instr->ir_call.p_des)
                    combine_map_self(p_instr->ir_call.p_des, p_map);
                break;
            case ir_load:
                combine_remap_operand(p_instr->ir_load.p_addr, p_map);
                if (p_instr->ir_load.p_offset)
                    combine_remap_operand(p_instr->ir_load.p_offset, p_map);
                combine_map_self(p_instr->ir_load.p_des, p_map);
                break;
            case ir_store:
                combine_remap_operand(p_instr->ir_store.p_addr, p_map);
                combine_remap_operand(p_instr->ir_store.p_src, p_map);
                if (p_instr->ir_store.p_offset)
                    combine_remap_operand(p_instr->ir_store.p_offset, p_map);
                break;
            case ir_gep:
                assert(0);
                break;
            }
            live_set_swap(p_instr->p_live_out, p_map, reg_num);
        }

        live_set_swap(p_basic_block->p_branch->p_live_in, p_map, reg_num);
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                combine_remap_operand(p_param, p_map);
            }
            break;
        case ir_cond_branch:
            combine_remap_operand(p_basic_block->p_branch->p_exp, p_map);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                combine_remap_operand(p_param, p_map);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->p_block_param->bb_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                combine_remap_operand(p_param, p_map);
            }
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                combine_remap_operand(p_basic_block->p_branch->p_exp, p_map);
            break;
        case ir_abort_branch:
            assert(0);
            break;
        }
        live_set_swap(p_basic_block->p_live_out, p_map, reg_num);
    }

    for (size_t i = 0; i < need_del_num; i++) {
        symbol_func_vreg_del(p_func, p_need_del[i]);
    }
    free(p_map);
    free(p_need_del);
}

void graph_alloca(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s) {
    p_graph_alloca_info p_info = graph_alloca_info_gen(reg_num_r, reg_num_s, p_func);
    liveness_analysis(p_info, p_func);
    set_func_live(p_info, p_func);

    graph_nodes_init(p_info->p_r_graph);
    graph_nodes_init(p_info->p_s_graph);
    pre_color(p_info, p_func);

    mcs_get_seqs(p_info->p_r_graph);
    check_chordal(p_info->p_r_graph);
    print_conflict_graph(p_info->p_r_graph);
    mcs_get_seqs(p_info->p_s_graph);
    check_chordal(p_info->p_s_graph);
    print_conflict_graph(p_info->p_s_graph);

    while (p_info->p_r_graph->color_num > p_info->p_r_graph->reg_num
        && p_info->p_s_graph->color_num > p_info->p_s_graph->reg_num) {
        maximum_clique(p_info->p_r_graph);
        choose_spill(p_info->p_r_graph);
        maximum_clique(p_info->p_s_graph);
        choose_spill(p_info->p_s_graph);
        graph_spill(p_info, p_func);
        graph_nodes_init(p_info->p_r_graph);
        print_conflict_graph(p_info->p_r_graph);
        mcs_get_seqs(p_info->p_r_graph);
        check_chordal(p_info->p_r_graph);

        graph_nodes_init(p_info->p_s_graph);
        print_conflict_graph(p_info->p_s_graph);
        mcs_get_seqs(p_info->p_s_graph);
        check_chordal(p_info->p_s_graph);
    }

    while (p_info->p_r_graph->color_num > p_info->p_r_graph->reg_num) {
        maximum_clique(p_info->p_r_graph);
        choose_spill(p_info->p_r_graph);
        graph_spill(p_info, p_func);
        graph_nodes_init(p_info->p_r_graph);
        print_conflict_graph(p_info->p_r_graph);
        mcs_get_seqs(p_info->p_r_graph);
        check_chordal(p_info->p_r_graph);
    }

    while (p_info->p_s_graph->color_num > p_info->p_s_graph->reg_num) {
        maximum_clique(p_info->p_s_graph);
        choose_spill(p_info->p_s_graph);
        graph_spill(p_info, p_func);
        graph_nodes_init(p_info->p_s_graph);
        print_conflict_graph(p_info->p_s_graph);
        mcs_get_seqs(p_info->p_s_graph);
        check_chordal(p_info->p_s_graph);
    }
    set_graph_color(p_info->p_r_graph);
    set_graph_color(p_info->p_s_graph);

    combine_mov(p_func);
    check_liveness(p_func);
    graph_alloca_info_drop(p_info);
    symbol_func_set_block_id(p_func);
    symbol_func_set_vreg_id(p_func);
}