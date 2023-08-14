#include <ir_opt/reg_alloca/graph_alloca/conflict_graph.h>
#include <ir_opt/reg_alloca/graph_alloca/graph_alloca.h>
#include <ir_manager/liveness.h>

#include <ir_gen.h>
#include <symbol_gen.h>

static inline void live_edge(p_ir_vreg_list p_live, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_live->vreg_list) {
        p_ir_vreg p_live_vreg = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        if (p_live_vreg == p_vreg) continue;
        add_reg_graph_edge(p_vreg, p_live_vreg);
    }
}

static inline void update_graph(p_conflict_graph p_graph, p_ir_vreg_list p_live, p_graph_node p_g_node) {
    neighbors_clear(p_g_node);
    live_edge(p_live, p_g_node->p_vreg);
}

static void new_store_front(p_conflict_graph p_graph, p_ir_vreg p_vreg, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_graph_node p_g_node = (p_graph_node) p_vreg->p_info;
    if (p_g_node->node_id >= p_graph->origin_node_num) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;
    p_ir_vreg_list p_live_in;
    if (list_head_alone(&p_basic_block->instr_list))
        p_live_in = p_basic_block->p_branch->p_live_in;
    else {
        p_ir_instr p_first_instr = list_entry(p_basic_block->instr_list.p_next, ir_instr, node);
        p_live_in = p_first_instr->p_live_in;
    }
    p_ir_instr p_store, p_assign;
    switch (p_o_node->spl_type) {
    case none:
        return;
    case reg_mem:
        p_store = ir_store_instr_gen(ir_operand_addr_gen(p_o_node->p_vmem, NULL, 0), ir_operand_vreg_gen(p_vreg), true);
        ir_basic_block_addinstr_head(p_basic_block, p_store);
        copy_vreg_list(p_store->p_live_out, p_live_in);
        copy_vreg_list(p_store->p_live_in, p_live_in);
        ir_vreg_list_add(p_store->p_live_in, p_vreg, 0);
        update_graph(p_graph, p_store->p_live_in, p_g_node);
        break;
    case reg_reg:
        p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_vreg_gen(p_o_node->p_def_node->p_vreg), p_o_node->p_spill_node->p_vreg);
        ir_basic_block_addinstr_head(p_basic_block, p_assign);
        copy_vreg_list(p_assign->p_live_out, p_live_in);
        copy_vreg_list(p_assign->p_live_in, p_live_in);
        ir_vreg_list_del(p_assign->p_live_in, p_o_node->p_spill_node->p_vreg);
        ir_vreg_list_add(p_assign->p_live_in, p_vreg, 0);
        copy_graph_neighbor(p_o_node->p_spill_node, p_o_node->p_def_node);
        update_graph(p_graph, p_assign->p_live_in, p_g_node);
        break;
    }
}

static void new_store_middle(p_conflict_graph p_graph, p_ir_vreg p_vreg, p_ir_instr p_instr, p_symbol_func p_func) {
    p_graph_node p_g_node = (p_graph_node) p_vreg->p_info;
    if (p_g_node->node_id >= p_graph->origin_node_num) return;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_g_node->node_id;

    p_ir_instr p_store, p_assign;
    switch (p_o_node->spl_type) {
    case none:
        return;
    case reg_mem:
        p_store = ir_store_instr_gen(ir_operand_addr_gen(p_o_node->p_vmem, NULL, 0), ir_operand_vreg_gen(p_vreg), true);
        ir_instr_add_next(p_store, p_instr);
        // 改变活跃集合
        copy_vreg_list(p_store->p_live_out, p_instr->p_live_out);
        ir_vreg_list_add(p_instr->p_live_out, p_vreg, 0);
        copy_vreg_list(p_store->p_live_in, p_instr->p_live_out);
        update_graph(p_graph, p_instr->p_live_out, p_o_node->p_def_node);
        break;
    case reg_reg:
        p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_vreg_gen(p_vreg), p_o_node->p_spill_node->p_vreg);
        ir_instr_add_next(p_assign, p_instr);
        copy_vreg_list(p_assign->p_live_out, p_instr->p_live_out);
        ir_vreg_list_del(p_instr->p_live_out, p_o_node->p_spill_node->p_vreg);
        ir_vreg_list_add(p_instr->p_live_out, p_vreg, 0);
        copy_vreg_list(p_assign->p_live_in, p_instr->p_live_out);
        copy_graph_neighbor(p_o_node->p_spill_node, p_o_node->p_def_node);
        update_graph(p_graph, p_instr->p_live_out, p_o_node->p_def_node);
        break;
    }
}

static p_ir_instr new_load(p_conflict_graph p_graph, p_ir_operand p_operand, p_ir_vreg_list p_live_in, p_ir_vreg_list p_live_out, p_symbol_func p_func) {
    if (p_operand->kind != reg) return NULL;
    p_graph_node p_vreg_g_node = (p_graph_node) p_operand->p_vreg->p_info;
    if (p_vreg_g_node->node_id >= p_graph->origin_node_num) return NULL;
    p_origin_graph_node p_o_node = p_graph->p_nodes + p_vreg_g_node->node_id;
    if (p_o_node->spl_type == none) return NULL;
    p_ir_vreg p_new_src = ir_vreg_copy(p_operand->p_vreg);
    symbol_func_vreg_add(p_func, p_new_src);
    p_graph_node p_g_node = graph_node_gen(p_new_src, p_graph);
    graph_node_list_add(p_o_node->p_use_spill_list, p_g_node);

    p_ir_instr p_load, p_assign;
    switch (p_o_node->spl_type) {
    case none:
        return NULL;
    case reg_mem:
        p_load = ir_load_instr_gen(ir_operand_addr_gen(p_o_node->p_vmem, NULL, 0), p_new_src, true);
        ir_operand_reset_vreg(p_operand, p_new_src);
        // 改变活跃集合和冲突图
        copy_vreg_list(p_load->p_live_in, p_live_in);
        ir_vreg_list_add(p_live_in, p_new_src, 0);
        copy_vreg_list(p_load->p_live_out, p_live_in);
        live_edge(p_load->p_live_out, p_new_src);
        return p_load;
    case reg_reg:
        p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_vreg_gen(p_o_node->p_spill_node->p_vreg), p_new_src);
        ir_operand_reset_vreg(p_operand, p_new_src);
        copy_vreg_list(p_assign->p_live_in, p_live_in);
        if (!if_in_vreg_list(p_live_out, p_o_node->p_spill_node->p_vreg))
            ir_vreg_list_del(p_live_in, p_o_node->p_spill_node->p_vreg);
        ir_vreg_list_add(p_live_in, p_new_src, 0);
        copy_vreg_list(p_assign->p_live_out, p_live_in);
        live_edge(p_assign->p_live_out, p_new_src);
        return p_assign;
    }
}
static inline void new_load_in_instr(p_conflict_graph p_graph, p_ir_operand p_operand, p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_instr p_new_instr = new_load(p_graph, p_operand, p_instr->p_live_in, p_instr->p_live_out, p_func);
    if (p_new_instr)
        ir_instr_add_prev(p_new_instr, p_instr);
}
static inline void new_load_in_branch(p_conflict_graph p_graph, p_ir_operand p_operand, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_ir_instr p_new_instr = new_load(p_graph, p_operand, p_basic_block->p_branch->p_live_in, p_basic_block->p_live_out, p_func);
    if (p_new_instr)
        ir_basic_block_addinstr_tail(p_basic_block, p_new_instr);
}
static inline void deal_live_set(p_conflict_graph p_graph, p_ir_vreg_list p_live) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_live->vreg_list) {
        p_ir_vreg_list_node p_vreg_list_node = list_entry(p_node, ir_vreg_list_node, node);
        p_graph_node p_g_node = (p_graph_node) p_vreg_list_node->p_vreg->p_info;
        if (p_g_node->node_id >= p_graph->origin_node_num) continue;
        switch ((p_graph->p_nodes + p_g_node->node_id)->spl_type) {
        case none:
            break;
        case reg_mem:
            ir_vreg_list_node_drop(p_vreg_list_node);
            break;
        case reg_reg:
            ir_vreg_list_node_set_vreg(p_vreg_list_node, (p_graph->p_nodes + p_g_node->node_id)->p_spill_node->p_vreg);
            break;
        }
    }
}

static inline void new_store_bb_phi_front(p_conflict_graph p_graph, p_ir_basic_block p_basic_block, p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->basic_block_phis) {
        p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
        new_store_front(p_graph, p_phi, p_basic_block, p_func);
    }
}
static inline void new_store_param_front(p_conflict_graph p_graph, p_symbol_func p_func) {
    // 处理形参
    p_ir_basic_block p_entry = p_func->p_entry_block;
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        new_store_front(p_graph, p_vreg, p_entry, p_func);
    }
}

void graph_spill(p_conflict_graph p_graph, p_symbol_func p_func) {
    p_list_head p_node;
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        deal_live_set(p_graph, p_basic_block->p_live_in);
        deal_live_set(p_graph, p_basic_block->p_live_out);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_live_set(p_graph, p_instr->p_live_in);
            deal_live_set(p_graph, p_instr->p_live_out);
            switch (p_instr->irkind) {
            case ir_binary:
                new_load_in_instr(p_graph, p_instr->ir_binary.p_src1, p_instr, p_func);
                new_load_in_instr(p_graph, p_instr->ir_binary.p_src2, p_instr, p_func);
                new_store_middle(p_graph, p_instr->ir_binary.p_des, p_instr, p_func);
                break;
            case ir_unary:
                new_load_in_instr(p_graph, p_instr->ir_unary.p_src, p_instr, p_func);
                new_store_middle(p_graph, p_instr->ir_unary.p_des, p_instr, p_func);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.param_list) {
                    p_ir_param p_param = list_entry(p_node, ir_param, node);
                    if(!p_param->is_in_mem)
                        new_load_in_instr(p_graph, p_param->p_param, p_instr, p_func);
                }
                if (p_instr->ir_call.p_des)
                    new_store_middle(p_graph, p_instr->ir_call.p_des, p_instr, p_func);
                break;
            case ir_load:
                new_load_in_instr(p_graph, p_instr->ir_load.p_addr, p_instr, p_func);
                new_store_middle(p_graph, p_instr->ir_load.p_des, p_instr, p_func);
                break;
            case ir_store:
                new_load_in_instr(p_graph, p_instr->ir_store.p_src, p_instr, p_func);
                new_load_in_instr(p_graph, p_instr->ir_store.p_addr, p_instr, p_func);
                break;
            case ir_gep:
                assert(0);
            }
        }

        deal_live_set(p_graph, p_basic_block->p_branch->p_live_in);
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->block_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load_in_branch(p_graph, p_param, p_basic_block, p_func);
            }
            break;
        case ir_cond_branch:
            new_load_in_branch(p_graph, p_basic_block->p_branch->p_exp, p_basic_block, p_func);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->block_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load_in_branch(p_graph, p_param, p_basic_block, p_func);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->block_param) {
                p_ir_operand p_param = list_entry(p_node, ir_bb_param, node)->p_bb_param;
                new_load_in_branch(p_graph, p_param, p_basic_block, p_func);
            }
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                new_load_in_branch(p_graph, p_basic_block->p_branch->p_exp, p_basic_block, p_func);
            break;
        case ir_abort_branch:
            assert(0);
        }
        new_store_bb_phi_front(p_graph, p_basic_block, p_func);
    }
    new_store_param_front(p_graph, p_func);
}

static inline void combine_remap_operand(p_ir_operand p_operand, p_ir_vreg *p_map) {
    if (p_operand->kind == reg) {
        size_t new_vreg_id = p_operand->p_vreg->id;
        size_t old_vreg_id = p_operand->p_vreg->id;

        while (p_map[new_vreg_id]->id != new_vreg_id) {
            new_vreg_id = p_map[new_vreg_id]->id;
            assert(new_vreg_id != old_vreg_id);
        }
        while (old_vreg_id != new_vreg_id) {
            size_t next_id = p_map[old_vreg_id]->id;
            p_map[old_vreg_id] = p_map[new_vreg_id];
            old_vreg_id = next_id;
        }
        assert(p_map[new_vreg_id]->id == new_vreg_id);
        ir_operand_reset_vreg(p_operand, p_map[new_vreg_id]);
    }
}

static inline void combine_map_self(p_ir_vreg p_vreg, p_ir_vreg *p_map) {
    p_map[p_vreg->id] = p_vreg;
}

static inline void live_set_swap(p_ir_vreg_list p_live, p_ir_vreg *p_map, size_t num) {
    p_list_head p_node, p_node_next;
    bool *if_have = malloc(sizeof(*if_have) * num);
    memset(if_have, false, sizeof(*if_have) * num);
    list_for_each_safe(p_node, p_node_next, &p_live->vreg_list) {
        p_ir_vreg_list_node p_live_node = list_entry(p_node, ir_vreg_list_node, node);
        p_ir_vreg p_new_vreg = p_map[p_live_node->p_vreg->id];
        if (if_have[p_new_vreg->id]) {
            ir_vreg_list_node_drop(p_live_node);
            continue;
        }
        if_have[p_new_vreg->id] = true;
        p_live_node->p_vreg = p_new_vreg;
    }
    free(if_have);
}

static void combine_mov(p_symbol_func p_func) {
    symbol_func_set_block_id(p_func);
    size_t reg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_ir_vreg *p_map = malloc(sizeof(void *) * reg_num);
    p_ir_vreg *p_need_del = malloc(sizeof(void *) * reg_num);
    size_t need_del_num = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        combine_map_self(p_vreg, p_map);
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        combine_map_self(p_vreg, p_map);
    }

    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind != ir_unary)
                continue;
            if (p_instr->ir_unary.op != ir_val_assign)
                continue;
            if (p_instr->ir_unary.p_src->kind != reg)
                continue;
            if (p_instr->ir_unary.p_src->p_vreg->if_float != p_instr->ir_unary.p_des->if_float)
                continue;
            if (p_instr->ir_unary.p_src->p_vreg->reg_id != p_instr->ir_unary.p_des->reg_id)
                continue;
            p_map[p_instr->ir_unary.p_des->id] = p_map[p_instr->ir_unary.p_src->p_vreg->id];
            p_need_del[need_del_num++] = p_instr->ir_unary.p_des;
            ir_instr_drop(p_instr);
        }
    }

    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        live_set_swap(p_basic_block->p_live_in, p_map, reg_num);

        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            live_set_swap(p_instr->p_live_in, p_map, reg_num);
            switch (p_instr->irkind) {
            case ir_binary:
                combine_remap_operand(p_instr->ir_binary.p_src1, p_map);
                combine_remap_operand(p_instr->ir_binary.p_src2, p_map);
                break;
            case ir_unary:
                combine_remap_operand(p_instr->ir_unary.p_src, p_map);
                break;
            case ir_call:
                list_for_each(p_node, &p_instr->ir_call.param_list) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    combine_remap_operand(p_param, p_map);
                }
                break;
            case ir_load:
                combine_remap_operand(p_instr->ir_load.p_addr, p_map);
                break;
            case ir_store:
                combine_remap_operand(p_instr->ir_store.p_addr, p_map);
                combine_remap_operand(p_instr->ir_store.p_src, p_map);
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
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->block_param) {
                p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
                assert(p_param->p_bb_param->used_type == bb_param_ptr && p_param->p_bb_param->p_bb_param == p_param);
                combine_remap_operand(p_param->p_bb_param, p_map);
            }
            break;
        case ir_cond_branch:
            combine_remap_operand(p_basic_block->p_branch->p_exp, p_map);
            list_for_each(p_node, &p_basic_block->p_branch->p_target_1->block_param) {
                p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
                assert(p_param->p_bb_param->used_type == bb_param_ptr && p_param->p_bb_param->p_bb_param == p_param);
                combine_remap_operand(p_param->p_bb_param, p_map);
            }
            list_for_each(p_node, &p_basic_block->p_branch->p_target_2->block_param) {
                p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
                assert(p_param->p_bb_param->used_type == bb_param_ptr && p_param->p_bb_param->p_bb_param == p_param);
                combine_remap_operand(p_param->p_bb_param, p_map);
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

static inline void delete_no_use_vreg(p_conflict_graph p_graph) {
    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        if ((p_graph->p_nodes + i)->need_delete) {
            symbol_func_vreg_del(p_graph->p_func, (p_graph->p_nodes + i)->p_def_node->p_vreg);
        }
    }
}
void liveness_info_gen_graph(p_liveness_info p_live_info)
{
    for (size_t i = 0; i < p_live_info->vreg_num; i++) {
        for (size_t j = i + 1; j < p_live_info->vreg_num; j++)
            if (p_live_info->graph_table[i][j])
                add_reg_graph_edge(p_live_info->p_vregs[i], p_live_info->p_vregs[j]);
    }
}

#include <stdio.h>
static inline void cal_spill_num(p_conflict_graph p_graph) {
    size_t spill_reg_num = 0;
    size_t spill_mem_num = 0;
    size_t spill_assign_instr_num = 0;
    size_t spill_mem_instr_num = 0;

    for (size_t i = 0; i < p_graph->origin_node_num; i++) {
        if ((p_graph->p_nodes + i)->p_spill_node) {
            spill_reg_num++;
            spill_assign_instr_num++;
            spill_assign_instr_num += (p_graph->p_nodes + i)->p_use_spill_list->num;
        }
        if ((p_graph->p_nodes + i)->p_vmem) {
            spill_mem_num++;
            spill_mem_instr_num++;
            spill_mem_instr_num += (p_graph->p_nodes + i)->p_use_spill_list->num;
        }
    }
    printf("used reg_num_r %ld, used_reg_num_s %ld\n", p_graph->p_func->use_reg_num_r, p_graph->p_func->use_reg_num_s);
    printf("spilled_reg_num: %ld, spilled_assign_instr_num %ld\n", spill_reg_num, spill_assign_instr_num);
    printf("spilled_mem_num: %ld, spilled_mem_instr_num %ld\n", spill_mem_num, spill_mem_instr_num);
}
void graph_alloca(p_symbol_func p_func, size_t reg_num_r, size_t reg_num_s) {
    size_t before_instr_num = p_func->instr_num;
    p_conflict_graph p_graph = conflict_graph_gen(reg_num_r, reg_num_s, p_func);
    p_liveness_info p_live_info = liveness_info_gen(p_func);
    liveness_analysis(p_live_info);
    liveness_info_gen_graph(p_live_info);
    mcs_get_seqs(p_graph);
    check_chordal(p_graph);
    maximum_clique(p_graph);
    get_color_num(p_graph);
    print_conflict_graph(p_graph);

    while (p_graph->color_num_r > p_graph->reg_num_r
        || p_graph->color_num_s > p_graph->reg_num_s) {
        choose_spill(p_graph);
        graph_spill(p_graph, p_func);
        graph_nodes_init(p_graph);
        print_conflict_graph(p_graph);
        mcs_get_seqs(p_graph);
        check_chordal(p_graph);
        maximum_clique(p_graph);
        get_color_num(p_graph);
    }
    set_graph_color(p_graph);
    adjust_graph_color(p_graph);
    delete_no_use_vreg(p_graph);

    combine_mov(p_func);
    liveness_info_drop(p_live_info);
    cal_spill_num(p_graph);
    conflict_graph_drop(p_graph);
    symbol_func_set_block_id(p_func);
    printf("before alloca instr num %s: %ld\n", p_func->name, before_instr_num);
    printf("after alloca instr num %s: %ld\n", p_func->name, p_func->instr_num);
}
