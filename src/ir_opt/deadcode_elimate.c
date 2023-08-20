#include <ir_gen.h>
#include <ir_manager/buildcdg.h>
#include <ir_opt/simplify_cfg.h>
#include <program/def.h>
#include <program/gen.h>
#include <symbol_gen.h>

typedef struct reg_info reg_info, *p_reg_info;
typedef struct reg_info_table reg_info_table, *p_reg_info_table;
typedef struct varray_info varray_info, *p_varray_info;

struct varray_info {
    p_ir_varray p_varray;
    p_varray_info p_prev;
};
struct reg_info {
    p_ir_vreg p_vreg;
    p_reg_info p_prev;
};

struct reg_info_table {
    p_symbol_func p_func;
    p_reg_info p_top;
    p_reg_info p_base;
    p_varray_info p_varray_top;
    p_varray_info p_varray_base;
    size_t vreg_num;
    size_t varray_num;
    size_t param_reg_num;
    bool if_aggressive;
};
static inline void reg_info_gen(p_reg_info p_info, p_ir_vreg p_vreg) {
    p_info->p_prev = NULL;
    p_info->p_vreg = p_vreg;
}
static inline void varray_info_gen(p_varray_info p_info, p_ir_varray p_varray) {
    p_info->p_prev = NULL;
    p_info->p_varray = p_varray;
    p_varray->p_info = p_info;
}
static inline void varray_info_set_useful(p_varray_info p_info) {
    p_info->p_prev = p_info;
}
static inline void push(p_ir_vreg p_vreg, p_reg_info_table reg_info_table) {
    if (!reg_info_table->p_top) {
        (reg_info_table->p_base + p_vreg->id)->p_prev = reg_info_table->p_base + p_vreg->id;
        reg_info_table->p_top = reg_info_table->p_base + p_vreg->id;
    }
    else if (!(reg_info_table->p_base + p_vreg->id)->p_prev) {
        (reg_info_table->p_base + p_vreg->id)->p_prev = reg_info_table->p_top;
        reg_info_table->p_top = reg_info_table->p_base + p_vreg->id;
    }
}
static inline void push_varray(p_ir_varray p_varray, p_reg_info_table reg_info_table) {
    p_varray_info p_info = p_varray->p_info;
    if (!reg_info_table->p_varray_top) {
        p_info->p_prev = p_info;
        reg_info_table->p_varray_top = p_info;
    }
    else if (!p_info->p_prev) {
        p_info->p_prev = reg_info_table->p_varray_top;
        reg_info_table->p_varray_top = p_info;
    }
}
static inline void deal_operand(p_ir_operand p_operand, p_reg_info_table reg_info_table) {
    if (p_operand && p_operand->kind == reg) {
        if (p_operand->p_vreg->def_type == func_param_def) {
            (reg_info_table->p_base + p_operand->p_vreg->id)->p_prev = (reg_info_table->p_base + p_operand->p_vreg->id);
            return;
        }
        push(p_operand->p_vreg, reg_info_table);
    }
}
static inline void deal_varray_use(p_ir_varray_use p_varray_use, p_reg_info_table reg_info_table) {
    if (!p_varray_use) return;
    if (p_varray_use->p_varray_use->varray_def_type == varray_func_def) {
        varray_info_set_useful(p_varray_use->p_varray_use->p_info);
        return;
    }
    if (p_varray_use->p_varray_use->varray_def_type == varray_global_def) {
        varray_info_set_useful(p_varray_use->p_varray_use->p_info);
        return;
    }
    push_varray(p_varray_use->p_varray_use, reg_info_table);
}
#include <ir_print.h>
static inline bool deal_instr_src(p_ir_instr p_instr, p_reg_info_table reg_info_table, p_symbol_func p_func) {
    p_list_head p_node;
    switch (p_instr->irkind) {
    case ir_call:
        if (p_instr->ir_call.p_func->p_side_effects
            && (!p_instr->ir_call.p_func->p_side_effects->input
                && !p_instr->ir_call.p_func->p_side_effects->output
                && !p_instr->ir_call.p_func->p_side_effects->stored_param_cnt
                && list_head_alone(&p_instr->ir_call.p_func->p_side_effects->stored_global))) {
            return false;
        }
        list_for_each(p_node, &p_instr->ir_call.param_list) {
            p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
            deal_operand(p_param, reg_info_table);
        }
        list_for_each(p_node, &p_instr->ir_call.varray_defs) {
            p_ir_varray_def_pair p_pair = list_entry(p_node, ir_varray_def_pair, node);
            deal_varray_use(p_pair->p_src, reg_info_table);
        }
        return true;
    case ir_store:
        if (!p_instr->ir_store.p_array_des) {
            deal_operand(p_instr->ir_store.p_addr, reg_info_table);
            deal_operand(p_instr->ir_store.p_src, reg_info_table);
            return true;
        }
        if (!p_instr->ir_store.p_array_des || !p_instr->ir_store.p_array_des->p_base->is_vmem || p_instr->ir_store.p_array_des->p_base->p_vmem_base->is_global) {
            deal_operand(p_instr->ir_store.p_addr, reg_info_table);
            deal_operand(p_instr->ir_store.p_src, reg_info_table);
            deal_varray_use(p_instr->ir_store.p_array_src, reg_info_table);
        }
        return true;
    default:
        if (!reg_info_table->if_aggressive) {
            deal_operand(ir_instr_get_src1(p_instr), reg_info_table);
            deal_operand(ir_instr_get_src2(p_instr), reg_info_table);
        }
        return false;
    }
}

static inline void set_cdg_prev_useful(p_reverse_dom_tree_info_list p_info_list, p_reg_info_table reg_info_table, p_ir_basic_block p_useful_block) {
    if (!reg_info_table->if_aggressive) return;
    p_reverse_dom_tree_info p_cdg_info = p_info_list->p_base + p_info_list->block2dfn_id[p_useful_block->block_id];
    p_list_head p_node;
    list_for_each(p_node, &p_cdg_info->p_cdg_prev->block_list) {
        p_ir_basic_block p_cdg_prev = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        assert(p_cdg_prev->p_branch->kind == ir_cond_branch);
        deal_operand(p_cdg_prev->p_branch->p_exp, reg_info_table);
    }
}

static inline void deal_block_call(p_reverse_dom_tree_info_list p_info_list, p_ir_basic_block_branch_target p_target, size_t param_index, p_reg_info_table reg_info_table) {
    size_t call_index = 0;
    p_list_head p_call_node;
    list_for_each(p_call_node, &p_target->block_param) {
        call_index++;
        if (call_index == param_index)
            deal_operand(list_entry(p_call_node, ir_bb_param, node)->p_bb_param, reg_info_table);
    }
    if (p_target->p_source_block->p_branch->kind == ir_cond_branch)
        deal_operand(p_target->p_source_block->p_branch->p_exp, reg_info_table);
    set_cdg_prev_useful(p_info_list, reg_info_table, p_target->p_source_block);
}

static inline void deal_block_param(p_reverse_dom_tree_info_list p_info_list, p_ir_vreg p_vreg, p_reg_info_table reg_info_table) {
    p_ir_basic_block p_def_block = p_vreg->p_bb_phi->p_basic_block;
    size_t param_index = 0;
    p_list_head p_param_node;
    list_for_each(p_param_node, &p_def_block->basic_block_phis) {
        param_index++;
        if (list_entry(p_param_node, ir_bb_phi, node)->p_bb_phi == p_vreg) {
            p_list_head p_prev_node;
            list_for_each(p_prev_node, &p_def_block->prev_branch_target_list) {
                p_ir_basic_block_branch_target p_prev_target = list_entry(p_prev_node, ir_branch_target_node, node)->p_target;
                deal_block_call(p_info_list, p_prev_target, param_index, reg_info_table);
            }
            break;
        }
    }
}

static inline void delete_block_call(p_ir_basic_block_branch_target p_target, size_t param_index) {
    size_t call_index = 0;
    p_list_head p_call_node;
    list_for_each(p_call_node, &p_target->block_param) {
        call_index++;
        if (call_index == param_index) {
            p_ir_bb_param p_bb_param = list_entry(p_call_node, ir_bb_param, node);
            ir_basic_block_branch_target_del_param(p_target, p_bb_param);
            break;
        }
    }
}

static inline void deal_varray_block_param(p_reverse_dom_tree_info_list p_info_list, p_ir_varray_bb_phi p_varray_bb_phi, p_reg_info_table reg_info_table) {
    p_list_head p_param_node;
    list_for_each(p_param_node, &p_varray_bb_phi->varray_param_list) {
        p_ir_varray_bb_param p_varray_param = list_entry(p_param_node, ir_varray_bb_param, phi_node);
        deal_varray_use(p_varray_param->p_varray_bb_param, reg_info_table);
        p_ir_basic_block p_source_block = p_varray_param->p_target->p_source_block;
        if (p_source_block->p_branch->kind == ir_cond_branch)
            deal_operand(p_source_block->p_branch->p_exp, reg_info_table);
        set_cdg_prev_useful(p_info_list, reg_info_table, p_source_block);
    }
}
static inline void deal_useful_instr(p_ir_instr p_instr, p_reg_info_table reg_info_table) {
    p_list_head p_node;
    switch (p_instr->irkind) {
    case ir_binary:
        deal_operand(p_instr->ir_binary.p_src1, reg_info_table);
        deal_operand(p_instr->ir_binary.p_src2, reg_info_table);
        break;
    case ir_unary:
        deal_operand(p_instr->ir_unary.p_src, reg_info_table);
        break;
    case ir_load:
        deal_operand(p_instr->ir_load.p_addr, reg_info_table);
        if (p_instr->ir_load.p_array_src)
            deal_varray_use(p_instr->ir_load.p_array_src, reg_info_table);
        break;
    case ir_gep:
        deal_operand(p_instr->ir_gep.p_addr, reg_info_table);
        deal_operand(p_instr->ir_gep.p_offset, reg_info_table);
        break;
    case ir_call:
        list_for_each(p_node, &p_instr->ir_call.param_list) {
            p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
            deal_operand(p_param, reg_info_table);
        }
        list_for_each(p_node, &p_instr->ir_call.varray_defs) {
            p_ir_varray_def_pair p_pair = list_entry(p_node, ir_varray_def_pair, node);
            deal_varray_use(p_pair->p_src, reg_info_table);
        }
        break;
    case ir_store:
        deal_operand(p_instr->ir_store.p_addr, reg_info_table);
        deal_operand(p_instr->ir_store.p_src, reg_info_table);
        deal_varray_use(p_instr->ir_store.p_array_src, reg_info_table);
        break;
    }
}
static inline void deal_reg_info_table(p_reverse_dom_tree_info_list p_info_list, p_reg_info_table reg_info_table) {
    while (reg_info_table->p_top || reg_info_table->p_varray_top) {
        if (reg_info_table->p_top) {
            bool if_visited_root = false;
            while (!(reg_info_table->p_top->p_prev == reg_info_table->p_top && if_visited_root)) {
                if (reg_info_table->p_top->p_prev == reg_info_table->p_top) if_visited_root = true;
                p_reg_info p_new_top = reg_info_table->p_top->p_prev;
                reg_info_table->p_top->p_prev = reg_info_table->p_top;
                p_ir_vreg p_vreg = reg_info_table->p_top->p_vreg;
                reg_info_table->p_top = p_new_top;
                p_ir_basic_block p_useful_block = NULL;
                if (p_vreg->def_type == bb_phi_def) {
                    deal_block_param(p_info_list, p_vreg, reg_info_table);
                    p_useful_block = p_vreg->p_bb_phi->p_basic_block;
                }
                else {
                    p_ir_instr p_def = p_vreg->p_instr_def;
                    p_useful_block = p_def->p_basic_block;
                    deal_useful_instr(p_def, reg_info_table);
                }
                set_cdg_prev_useful(p_info_list, reg_info_table, p_useful_block);
            }
            reg_info_table->p_top = NULL;
        }
        if (reg_info_table->p_varray_top) {
            bool if_visited_root = false;
            while (!(reg_info_table->p_varray_top->p_prev == reg_info_table->p_varray_top && if_visited_root)) {
                if (reg_info_table->p_varray_top->p_prev == reg_info_table->p_varray_top) if_visited_root = true;
                p_varray_info p_new_top = reg_info_table->p_varray_top->p_prev;
                reg_info_table->p_varray_top->p_prev = reg_info_table->p_varray_top;
                p_ir_varray p_varray = reg_info_table->p_varray_top->p_varray;
                reg_info_table->p_varray_top = p_new_top;
                p_ir_basic_block p_useful_block = NULL;
                if (!p_varray->p_instr_def) continue; // 在某些时候删指令而不能删varray
                switch (p_varray->varray_def_type) {
                case varray_bb_phi_def:
                    deal_varray_block_param(p_info_list, p_varray->p_varray_bb_phi, reg_info_table);
                    p_useful_block = p_varray->p_varray_bb_phi->p_basic_block;
                    break;
                case varray_instr_def:
                    p_useful_block = p_varray->p_instr_def->p_basic_block;
                    deal_useful_instr(p_varray->p_instr_def, reg_info_table);
                    break;
                case varray_def_pair_def:
                    assert(p_varray->p_pair->p_instr->irkind == ir_call);
                    p_useful_block = p_varray->p_pair->p_instr->p_basic_block;
                    deal_useful_instr(p_varray->p_pair->p_instr, reg_info_table);
                    break;
                default:
                    assert(0);
                }
                set_cdg_prev_useful(p_info_list, reg_info_table, p_useful_block);
            }
            reg_info_table->p_varray_top = NULL;
        }
    }
}
static inline bool if_vreg_useful(p_ir_vreg p_vreg, p_reg_info_table reg_info_table) {
    return (reg_info_table->p_base + p_vreg->id)->p_prev != NULL;
}
static inline bool if_varray_useful(p_ir_varray p_varray) {
    return ((p_varray_info) p_varray->p_info)->p_prev != NULL;
}
static inline void delete_all(p_reverse_dom_tree_info_list p_info_list, p_reg_info_table reg_info_table, p_symbol_func p_func) {
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_node, p_node_next;
        size_t param_index = 0;
        list_for_each_safe(p_node, p_node_next, &p_block->basic_block_phis) {
            param_index++;
            p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
            if (!(if_vreg_useful(p_phi->p_bb_phi, reg_info_table))) {
                p_list_head p_prev_node;
                list_for_each(p_prev_node, &p_block->prev_branch_target_list) {
                    p_ir_basic_block_branch_target p_prev_target = list_entry(p_prev_node, ir_branch_target_node, node)->p_target;
                    delete_block_call(p_prev_target, param_index);
                }
                param_index--;
                ir_basic_block_del_phi(p_block, p_phi);
            }
        }
        list_for_each_safe(p_node, p_node_next, &p_block->varray_basic_block_phis) {
            p_ir_varray_bb_phi p_phi = list_entry(p_node, ir_varray_bb_phi, node);
            if (!if_varray_useful(p_phi->p_varray_phi)) {
                ir_basic_block_del_varray_phi(p_block, p_phi);
            }
        }
        list_for_each_safe(p_node, p_node_next, &p_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            bool if_useful = false;
            switch (p_instr->irkind) {
            case ir_store:
                if (!p_instr->ir_store.p_array_des) {
                    if_useful = true;
                    break;
                }
                if (if_varray_useful(p_instr->ir_store.p_array_des))
                    if_useful = true;
                if (!p_instr->ir_store.p_array_des->p_base->is_vmem || p_instr->ir_store.p_array_des->p_base->p_vmem_base->is_global) {
                    if_useful = true;
                    varray_info_set_useful(p_instr->ir_store.p_array_des->p_info);
                }
                break;
            case ir_call:
                if (!p_instr->ir_call.p_func->p_side_effects
                    || (p_instr->ir_call.p_func->p_side_effects->input
                        || p_instr->ir_call.p_func->p_side_effects->output
                        || p_instr->ir_call.p_func->p_side_effects->stored_param_cnt
                        || !list_head_alone(&p_instr->ir_call.p_func->p_side_effects->stored_global))) {
                    if_useful = true;
                }
                if (p_instr->ir_call.p_des) {
                    if (!if_vreg_useful(p_instr->ir_call.p_des, reg_info_table)) {
                        ir_set_call_instr_des(p_instr, NULL);
                    }
                    else {
                        if_useful = true;
                        // break;
                    }
                }
                p_list_head p_node;
                list_for_each(p_node, &p_instr->ir_call.varray_defs) {
                    p_ir_varray_def_pair p_pair = list_entry(p_node, ir_varray_def_pair, node);
                    if (!p_pair->p_des)
                        continue;
                    if (if_varray_useful(p_pair->p_des)) {
                        if_useful = true;
                    }
                    else {
                        ir_varray_def_pair_set_des(p_pair, NULL);
                    }
                }
                break;
            case ir_load:
                if (if_vreg_useful(p_instr->ir_load.p_des, reg_info_table))
                    if_useful = true;
                break;
            case ir_binary:
                if (if_vreg_useful(p_instr->ir_binary.p_des, reg_info_table))
                    if_useful = true;
                break;
            case ir_unary:
                if (if_vreg_useful(p_instr->ir_unary.p_des, reg_info_table))
                    if_useful = true;
                break;
            case ir_gep:
                if (if_vreg_useful(p_instr->ir_gep.p_des, reg_info_table))
                    if_useful = true;
                break;
            }
            if (!if_useful) {
                ir_instr_drop(p_instr);
            }
        }
        if (p_block->p_branch->p_exp) { // 需要删除条件语句
            if (p_block->p_branch->p_exp->kind == reg && !(reg_info_table->p_base + p_block->p_branch->p_exp->p_vreg->id)->p_prev) {
                assert(p_block->p_branch->kind == ir_cond_branch);
                assert(reg_info_table->if_aggressive);
                ir_basic_block_branch_target_drop(p_block, p_block->p_branch->p_target_1);
                ir_basic_block_branch_target_drop(p_block, p_block->p_branch->p_target_2);
                ir_operand_drop(p_block->p_branch->p_exp);
                p_block->p_branch->p_target_1 = p_block->p_branch->p_target_2 = NULL;
                p_block->p_branch->p_exp = NULL;
                ir_basic_block_set_br(p_block, p_info_list->p_base[p_info_list->block2dfn_id[p_block->block_id]].reverse_dom_parent);
            }
        }
    }
}
static inline p_reg_info_table reg_info_table_gen(bool if_aggressive, p_symbol_func p_func) {
    p_reg_info_table reg_info_table = malloc(sizeof(*reg_info_table));
    reg_info_table->p_func = p_func;
    reg_info_table->if_aggressive = if_aggressive;
    reg_info_table->param_reg_num = p_func->param_reg_cnt;
    reg_info_table->vreg_num = p_func->param_reg_cnt + p_func->vreg_cnt;
    reg_info_table->p_base = malloc(sizeof(*reg_info_table->p_base) * reg_info_table->vreg_num);
    reg_info_table->p_top = NULL;
    reg_info_table->p_varray_base = malloc(sizeof(*reg_info_table->p_varray_base) * (p_func->varray_num + p_func->p_program->varray_num));
    reg_info_table->p_varray_top = NULL;

    size_t i = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_program->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_list_head p_vmem_node;
        list_for_each(p_vmem_node, &p_var->p_base->varray_list) {
            p_ir_varray p_varray = list_entry(p_vmem_node, ir_varray, node);
            if (!p_varray->p_instr_def) continue;
            switch (p_varray->varray_def_type) {
            case varray_func_def:
            case varray_global_def:
                if (p_varray->p_func != reg_info_table->p_func)
                    continue;
                break;
            case varray_instr_def:
                if (p_varray->p_instr_def->p_basic_block->p_func != reg_info_table->p_func)
                    continue;
                break;
            case varray_bb_phi_def:
                if (p_varray->p_varray_bb_phi->p_basic_block->p_func != reg_info_table->p_func)
                    continue;
                break;
            case varray_def_pair_def:
                if (p_varray->p_pair->p_instr->p_basic_block->p_func != reg_info_table->p_func)
                    continue;
                break;
            }
            varray_info_gen(reg_info_table->p_varray_base + i, p_varray);
            i++;
        }
    }

    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, ir_param_vmem_base, node)->p_param_base;
        p_list_head p_vmem_node;
        list_for_each(p_vmem_node, &p_vmem_base->varray_list) {
            p_ir_varray p_varray = list_entry(p_vmem_node, ir_varray, node);
            varray_info_gen(reg_info_table->p_varray_base + i, p_varray);
            i++;
            assert(p_varray);
        }
    }
    list_for_each(p_node, &p_func->param) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        p_list_head p_vmem_node;
        list_for_each(p_vmem_node, &p_vmem_base->varray_list) {
            p_ir_varray p_varray = list_entry(p_vmem_node, ir_varray, node);
            varray_info_gen(reg_info_table->p_varray_base + i, p_varray);
            i++;
            assert(p_varray);
        }
    }
    list_for_each(p_node, &p_func->variable) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        p_list_head p_vmem_node;
        list_for_each(p_vmem_node, &p_vmem_base->varray_list) {
            p_ir_varray p_varray = list_entry(p_vmem_node, ir_varray, node);
            varray_info_gen(reg_info_table->p_varray_base + i, p_varray);
            assert(p_varray);
            i++;
        }
    }
    reg_info_table->varray_num = i;
    i = 0;
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        reg_info_gen(reg_info_table->p_base + p_vreg->id, p_vreg);
        i++;
    }
    assert(i == p_func->vreg_cnt);
    return reg_info_table;
}

static inline void reg_info_table_clear(p_reg_info_table reg_info_table) {
    for (size_t j = reg_info_table->param_reg_num; j < reg_info_table->vreg_num; j++) {
        if (!(reg_info_table->p_base + j)->p_prev) {
            symbol_func_vreg_del(reg_info_table->p_func, (reg_info_table->p_base + j)->p_vreg);
        }
    }
    for (size_t j = 0; j < reg_info_table->varray_num; j++) {
        p_varray_info p_info = reg_info_table->p_varray_base + j;
        if (!p_info->p_prev) {
            ir_varray_drop(p_info->p_varray);
        }
    }
    symbol_func_set_block_id(reg_info_table->p_func);
    if (reg_info_table->varray_num) {
        symbol_func_clear_varible(reg_info_table->p_func);
        program_clear_varible(reg_info_table->p_func->p_program);
    }
}
static inline void ir_dead_code_elimate_func(p_symbol_func p_func, bool if_aggressive) {
    p_reverse_dom_tree_info_list p_info_list = ir_build_cdg_func(p_func);
    p_reg_info_table reg_info_table = reg_info_table_gen(if_aggressive, p_func);
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block, node);
        bool if_useful = false;
        p_list_head p_instr_node, p_next;
        list_for_each_safe(p_instr_node, p_next, &p_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            ir_instr_print(p_instr);
            if_useful |= deal_instr_src(p_instr, reg_info_table, p_func);
        }

        if (p_block->p_branch->kind == ir_ret_branch)
            deal_operand(p_block->p_branch->p_exp, reg_info_table);
        if (!if_aggressive && p_block->p_branch->kind == ir_cond_branch)
            deal_operand(p_block->p_branch->p_exp, reg_info_table);
        if (if_useful)
            set_cdg_prev_useful(p_info_list, reg_info_table, p_block);
    }

    deal_reg_info_table(p_info_list, reg_info_table);

    delete_all(p_info_list, reg_info_table, p_func);
    reg_info_table_clear(reg_info_table);
    free(reg_info_table->p_base);
    free(reg_info_table->p_varray_base);
    free(reg_info_table);
    reverse_dom_tree_info_list_drop(p_info_list);
}
static inline void _ir_deadcode_elimate_pass(p_program p_ir, bool if_aggressive) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        symbol_func_set_block_id(p_func);
        ir_dead_code_elimate_func(p_func, if_aggressive);
        symbol_func_set_varible_id(p_func);
    }
}

void ir_deadcode_elimate_pass(p_program p_ir, bool if_aggressive) {
    bool if_del;
    do {
        _ir_deadcode_elimate_pass(p_ir, if_aggressive);
        if_del = ir_simplify_cfg_pass(p_ir);
    } while (if_del);
}

void ir_func_deadcode_elimate_just(p_symbol_func p_func) {
    assert(p_func->p_entry_block);
    symbol_func_set_block_id(p_func);
    ir_dead_code_elimate_func(p_func, false);
    symbol_func_set_varible_id(p_func);
}
