#include <ir_gen.h>
#include <ir_opt/simplify_cfg.h>
#include <program/def.h>
#include <symbol_gen/func.h>

typedef struct reg_info reg_info, *p_reg_info;

struct reg_info {
    p_ir_vreg p_vreg;
    p_reg_info p_prev;
};

static inline void reg_info_gen(p_reg_info p_info, p_ir_vreg p_vreg) {
    p_info->p_prev = NULL;
    p_info->p_vreg = p_vreg;
}

static inline p_reg_info push(p_ir_vreg p_vreg, p_reg_info reg_info_table, p_reg_info p_top) {
    if (!p_top) {
        (reg_info_table + p_vreg->id)->p_prev = reg_info_table + p_vreg->id;
        return reg_info_table + p_vreg->id;
    }
    else if (!(reg_info_table + p_vreg->id)->p_prev) {
        (reg_info_table + p_vreg->id)->p_prev = p_top;
        return reg_info_table + p_vreg->id;
    }
    return p_top;
}

static inline p_reg_info deal_operand(p_ir_operand p_operand, p_reg_info reg_info_table, p_reg_info p_top, p_symbol_func p_func) {
    if (p_operand && p_operand->kind == reg) {
        if (p_operand->p_vreg->id < p_func->param_reg_cnt) return p_top;
        return push(p_operand->p_vreg, reg_info_table, p_top);
    }
    return p_top;
}

static inline p_reg_info deal_instr_src(p_ir_instr p_instr, p_reg_info reg_info_table, p_reg_info p_top, p_symbol_func p_func, bool if_aggressive) {
    p_list_head p_node;
    switch (p_instr->irkind) {
    case ir_call:
        list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
            p_top = deal_operand(list_entry(p_node, ir_param, node)->p_param, reg_info_table, p_top, p_func);
        }
        if (p_instr->ir_call.p_des)
            (reg_info_table + p_instr->ir_call.p_des->id)->p_prev = (reg_info_table + p_instr->ir_call.p_des->id);
        break;
    case ir_store:
        p_top = deal_operand(p_instr->ir_store.p_addr, reg_info_table, p_top, p_func);
        p_top = deal_operand(p_instr->ir_store.p_src, reg_info_table, p_top, p_func);
        break;
    default:
        if (!if_aggressive) {
            p_top = deal_operand(ir_instr_get_src1(p_instr), reg_info_table, p_top, p_func);
            p_top = deal_operand(ir_instr_get_src2(p_instr), reg_info_table, p_top, p_func);
        }
        break;
    }
    return p_top;
}

static inline p_reg_info deal_block_call(p_ir_basic_block_branch_target p_target, size_t param_index, p_reg_info reg_info_table, p_reg_info p_top, p_symbol_func p_func) {
    size_t call_index = 0;
    p_list_head p_call_node;
    list_for_each(p_call_node, &p_target->block_param) {
        call_index++;
        if (call_index == param_index)
            return deal_operand(list_entry(p_call_node, ir_bb_param, node)->p_bb_param, reg_info_table, p_top, p_func);
    }
    return p_top;
}

static inline p_reg_info deal_block_param(p_ir_vreg p_vreg, p_reg_info reg_info_table, p_reg_info p_top, p_symbol_func p_func) {
    p_ir_basic_block p_def_block = p_vreg->p_bb_def;
    size_t param_index = 0;
    p_list_head p_param_node;
    list_for_each(p_param_node, &p_def_block->basic_block_phis->bb_phi) {
        param_index++;
        if (list_entry(p_param_node, ir_bb_phi, node)->p_bb_phi == p_vreg) {
            p_list_head p_prev_node;
            list_for_each(p_prev_node, &p_def_block->prev_basic_block_list) {
                p_ir_basic_block p_prev_block = list_entry(p_prev_node, ir_basic_block_list_node, node)->p_basic_block;
                if (p_prev_block->p_branch->p_target_1 && p_prev_block->p_branch->p_target_1->p_block == p_def_block)
                    p_top = deal_block_call(p_prev_block->p_branch->p_target_1, param_index, reg_info_table, p_top, p_func);
                else
                    p_top = deal_block_call(p_prev_block->p_branch->p_target_2, param_index, reg_info_table, p_top, p_func);
            }
            break;
        }
    }
    return p_top;
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

static inline void _ir_deadcode_elimate_pass(p_program p_ir, bool if_aggressive) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        size_t vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
        p_reg_info p_top = NULL;
        p_reg_info reg_info_table = malloc(vreg_num * sizeof(*reg_info_table));
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_block = list_entry(p_block_node, ir_basic_block, node);
            p_list_head p_node;
            list_for_each(p_node, &p_block->basic_block_phis->bb_phi) {
                p_ir_vreg p_vreg = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
                reg_info_gen(reg_info_table + p_vreg->id, p_vreg);
            }

            p_list_head p_instr_node;
            list_for_each(p_instr_node, &p_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
                p_ir_vreg p_des = ir_instr_get_des(p_instr);
                if (p_des)
                    reg_info_gen(reg_info_table + p_des->id, p_des);
                p_top = deal_instr_src(p_instr, reg_info_table, p_top, p_func, if_aggressive);
            }

            if (p_block->p_branch->kind != ir_br_branch)
                p_top = deal_operand(p_block->p_branch->p_exp, reg_info_table, p_top, p_func);
        }

        bool if_visited_root = false;
        while (p_top && !(p_top->p_prev == p_top && if_visited_root)) {
            if (p_top->p_prev == p_top) if_visited_root = true;
            p_reg_info p_new_top = p_top->p_prev;
            p_top->p_prev = p_top;
            p_ir_vreg p_vreg = p_top->p_vreg;
            p_top = p_new_top;
            if (p_vreg->is_bb_param)
                p_top = deal_block_param(p_vreg, reg_info_table, p_top, p_func);
            else {
                p_top = deal_operand(ir_instr_get_src1(p_vreg->p_instr_def), reg_info_table, p_top, p_func);
                p_top = deal_operand(ir_instr_get_src2(p_vreg->p_instr_def), reg_info_table, p_top, p_func);
            }
        }

        for (size_t j = p_func->param_reg_cnt; j < vreg_num; j++) {
            if (!(reg_info_table + j)->p_prev) {
                if ((reg_info_table + j)->p_vreg->is_bb_param) {
                    p_ir_basic_block p_def_block = (reg_info_table + j)->p_vreg->p_bb_def;
                    size_t param_index = 0;
                    p_list_head p_param_node;
                    list_for_each(p_param_node, &p_def_block->basic_block_phis->bb_phi) {
                        param_index++;
                        p_ir_bb_phi p_bb_phi = list_entry(p_param_node, ir_bb_phi, node);
                        if (p_bb_phi->p_bb_phi == (reg_info_table + j)->p_vreg) {
                            p_list_head p_prev_node;
                            list_for_each(p_prev_node, &p_def_block->prev_basic_block_list) {
                                p_ir_basic_block p_prev_block = list_entry(p_prev_node, ir_basic_block_list_node, node)->p_basic_block;
                                if (p_prev_block->p_branch->p_target_1 && p_prev_block->p_branch->p_target_1->p_block == p_def_block)
                                    delete_block_call(p_prev_block->p_branch->p_target_1, param_index);
                                else
                                    delete_block_call(p_prev_block->p_branch->p_target_2, param_index);
                            }
                            list_del(&p_bb_phi->node);
                            free(p_bb_phi);
                            break;
                        }
                    }
                }
                else {
                    ir_instr_drop((reg_info_table + j)->p_vreg->p_instr_def);
                }
            }
        }
        for (size_t j = p_func->param_reg_cnt; j < vreg_num; j++) {
            if (!(reg_info_table + j)->p_prev) {
                symbol_func_vreg_del(p_func, (reg_info_table + j)->p_vreg);
            }
        }
        symbol_func_set_block_id(p_func);
        free(reg_info_table);
    }
}
void ir_deadcode_elimate_pass(p_program p_ir, bool if_aggressive) {
    bool if_del;
    do {
        _ir_deadcode_elimate_pass(p_ir, if_aggressive);
        if_del = ir_simplify_cfg_pass(p_ir);
    } while (if_del);
}
