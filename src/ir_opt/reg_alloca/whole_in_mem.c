#include <ir_gen.h>
#include <ir_opt/reg_alloca/whole_in_mem.h>
#include <symbol_gen.h>

static inline void set_reg_id(p_inmem_alloca_info p_info, p_ir_vreg p_vreg) {
    assert(p_info->current_reg_r < p_info->reg_num_r);
    assert(p_info->current_reg_s < p_info->reg_num_s);
    if (p_vreg->if_float)
        p_vreg->reg_id = p_info->current_reg_s++;
    else
        p_vreg->reg_id = p_info->current_reg_r++;
}

static inline p_ir_vreg copy_vreg(p_inmem_alloca_info p_info, p_ir_vreg p_vreg) {
    p_ir_vreg p_new_vreg = ir_vreg_copy(p_vreg);
    symbol_func_vreg_add(p_info->p_func, p_new_vreg);
    return p_new_vreg;
}

static void new_load_operand(p_inmem_alloca_info p_info, p_ir_instr p_instr, p_ir_operand p_operand) {
    assert(p_operand);
    if (p_operand->kind == reg) {
        assert(p_info->pp_vmem[p_operand->p_vreg->id]);
        p_ir_vreg p_new_src = copy_vreg(p_info, p_operand->p_vreg);
        p_ir_operand p_vmem_operand = ir_operand_addr_gen(p_info->pp_vmem[p_operand->p_vreg->id]);
        p_ir_instr p_load = ir_load_instr_gen(p_vmem_operand, p_new_src, true);
        set_reg_id(p_info, p_new_src);
        list_add_prev(&p_load->node, &p_instr->node);
        p_operand->p_vreg = p_new_src;
    }
}

static inline p_symbol_var reg2mem(p_inmem_alloca_info p_info, p_ir_vreg p_vreg) {
    assert(!p_info->pp_vmem[p_vreg->id]);
    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
    symbol_func_add_variable(p_info->p_func, p_vmem);
    p_info->pp_vmem[p_vreg->id] = p_vmem;
    return p_vmem;
}
static inline void new_store_vreg(p_inmem_alloca_info p_info, p_ir_instr p_instr, p_ir_vreg p_vreg) {
    p_symbol_var p_vmem = reg2mem(p_info, p_vreg);
    set_reg_id(p_info, p_vreg);
    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem), ir_operand_vreg_gen(p_vreg), true);
    list_add_next(&p_store->node, &p_instr->node);
}
static void deal_binary_instr(p_inmem_alloca_info p_info, p_ir_instr p_instr) {
    assert(p_instr->irkind == ir_binary);
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    switch (p_binary_instr->op) {
    case ir_add_op:
    case ir_sub_op:
    case ir_mul_op:
    case ir_div_op:
        new_load_operand(p_info, p_instr, p_binary_instr->p_src1);
        new_load_operand(p_info, p_instr, p_binary_instr->p_src2);
        new_store_vreg(p_info, p_instr, p_binary_instr->p_des);
        break;
    case ir_eq_op:
    case ir_neq_op:
    case ir_l_op:
    case ir_leq_op:
    case ir_g_op:
    case ir_geq_op:
        new_load_operand(p_info, p_instr, p_binary_instr->p_src1);
        new_load_operand(p_info, p_instr, p_binary_instr->p_src2);
        new_store_vreg(p_info, p_instr, p_binary_instr->p_des);
        break;
    case ir_mod_op:
        assert(0);
        break;
    }
}

static void deal_unary_instr(p_inmem_alloca_info p_info, p_ir_instr p_instr) {
    assert(p_instr->irkind == ir_unary);
    p_ir_unary_instr p_unary_instr = &p_instr->ir_unary;
    switch (p_unary_instr->op) {
    case ir_minus_op:
    case ir_val_assign:
    case ir_i2f_op:
    case ir_f2i_op:
        new_load_operand(p_info, p_instr, p_unary_instr->p_src);
        new_store_vreg(p_info, p_instr, p_unary_instr->p_des);
        break;
    }
}

static void deal_instr(p_inmem_alloca_info p_info, p_ir_instr p_instr) {
    size_t current_reg_r = p_info->current_reg_r;
    size_t current_reg_s = p_info->current_reg_s;
    switch (p_instr->irkind) {
    case ir_binary:
        deal_binary_instr(p_info, p_instr);
        break;
    case ir_unary:
        deal_unary_instr(p_info, p_instr);
        break;
    case ir_call:
        assert(current_reg_r == 0);
        assert(current_reg_s == 0);
        p_list_head p_node;
        list_for_each(p_node, &p_instr->ir_call.p_param_list->param) {
            p_ir_param p_param = list_entry(p_node, ir_param, node);
            new_load_operand(p_info, p_instr, p_param->p_param);
        }
        if (p_instr->ir_call.p_des) {
            new_store_vreg(p_info, p_instr, p_instr->ir_call.p_des);
            p_instr->ir_call.p_des->reg_id = 0;
        }
        break;
    case ir_store:
        new_load_operand(p_info, p_instr, p_instr->ir_store.p_src);
        new_load_operand(p_info, p_instr, p_instr->ir_store.p_addr);
        break;
    case ir_load:
        new_store_vreg(p_info, p_instr, p_instr->ir_load.p_des);
        new_load_operand(p_info, p_instr, p_instr->ir_load.p_addr);
        break;
    case ir_gep:
        assert(0);
        break;
    }
    p_info->current_reg_r = current_reg_r;
    p_info->current_reg_s = current_reg_s;
}

static inline void deal_block_param(p_inmem_alloca_info p_info, p_ir_basic_block p_block, p_ir_bb_param_list p_param_list) {
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->bb_param) {
        p_ir_param p_param = list_entry(p_node, ir_param, node);
        if (p_param->p_param)
            new_load_operand(p_info, list_entry(&p_block->instr_list, ir_instr, node), p_param->p_param);
    }
}

static inline p_inmem_alloca_info inmem_alloca_info_gen(size_t reg_num_r, size_t reg_num_s, p_symbol_func p_func) {
    p_inmem_alloca_info p_info = malloc(sizeof(*p_info));
    p_info->reg_num_r = reg_num_r;
    p_info->reg_num_s = reg_num_s;
    size_t origin_vreg_num = p_func->vreg_cnt + p_func->param_reg_cnt;
    p_info->pp_vmem = malloc(sizeof(void *) * origin_vreg_num);
    p_info->p_func = p_func;
    memset(p_info->pp_vmem, 0, origin_vreg_num * sizeof(void *));
    return p_info;
}

static inline void inmem_alloca_info_drop(p_inmem_alloca_info p_info) {
    free(p_info->pp_vmem);
    free(p_info);
}

void whole_in_mem_alloca(p_symbol_func p_func, size_t reg_r_num, size_t reg_s_num) {
    p_inmem_alloca_info p_info = inmem_alloca_info_gen(reg_r_num, reg_s_num, p_func);
    size_t current_reg_r = p_info->current_reg_r = 0;
    size_t current_reg_s = p_info->current_reg_s = 0;

    p_list_head p_node;
    p_list_head p_instr_node;
    // 处理形参
    p_ir_basic_block p_entry = list_entry(p_func->block.p_next, ir_basic_block, node);
    p_instr_node = p_entry->instr_list.p_next;
    p_ir_instr p_head_instr = list_entry(&p_entry->instr_list, ir_instr, node);
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param = list_entry(p_node, ir_vreg, node);
        new_store_vreg(p_info, p_head_instr, p_param);
    }
    p_info->current_reg_r = current_reg_r;
    p_info->current_reg_s = current_reg_s;

    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        if (!p_instr_node)
            p_instr_node = p_basic_block->instr_list.p_next;
        p_head_instr = list_entry(&p_basic_block->instr_list, ir_instr, node);
        list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
            p_ir_vreg p_phi = list_entry(p_node, ir_bb_phi, node)->p_bb_phi;
            new_store_vreg(p_info, p_head_instr, p_phi);
        }
        p_info->current_reg_r = current_reg_r;
        p_info->current_reg_s = current_reg_s;

        p_list_head p_instr_node_next;
        while (p_instr_node != &p_basic_block->instr_list) {
            p_instr_node_next = p_instr_node->p_next;
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            deal_instr(p_info, p_instr);
            p_instr_node = p_instr_node_next;
        }
        switch (p_basic_block->p_branch->kind) {
        case ir_br_branch:
            deal_block_param(p_info, p_basic_block, p_basic_block->p_branch->p_target_1->p_block_param);
            break;
        case ir_abort_branch:
            break;
        case ir_cond_branch:
            deal_block_param(p_info, p_basic_block, p_basic_block->p_branch->p_target_1->p_block_param);
            deal_block_param(p_info, p_basic_block, p_basic_block->p_branch->p_target_2->p_block_param);
            break;
        case ir_ret_branch:
            if (p_basic_block->p_branch->p_exp)
                new_load_operand(p_info, list_entry(&p_basic_block->instr_list, ir_instr, node), p_basic_block->p_branch->p_exp);
            break;
        }
        p_instr_node = NULL;
        p_info->current_reg_r = current_reg_r;
        p_info->current_reg_s = current_reg_s;
    }
    inmem_alloca_info_drop(p_info);
    symbol_func_set_block_id(p_func);
}
