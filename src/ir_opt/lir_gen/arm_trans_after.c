#include <ir_gen.h>
#include <ir_opt/lir_gen/arm_standard.h>
#include <ir_opt/lir_gen/arm_trans_after.h>
#include <program/def.h>
#include <symbol_gen.h>

// 对齐到Align的整数倍
static inline size_t alignTo(size_t N, size_t Align) {
    // (0,Align]返回Align
    return (N + Align - 1) / Align * Align;
}
static inline void stack_alloc(p_symbol_func p_func) {
    p_list_head p_node;
    size_t stack_size = p_func->inner_stack_size;
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_vmem->stack_offset = stack_size;
        stack_size += p_vmem->p_type->size * basic_type_get_size(p_vmem->p_type->basic);
    }
    size_t save_size = (p_func->save_reg_r_num + p_func->save_reg_s_num + 1) * basic_type_get_size(type_i32);
    p_func->stack_size = alignTo(stack_size + save_size, 16) - save_size;
    stack_size = p_func->stack_size + save_size;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_vmem->stack_offset = stack_size;
        stack_size += basic_type_get_size(p_vmem->p_type->basic);
    }
}

static inline void remap_reg_id(p_symbol_func p_func) {
    size_t r_map[13] = { 0, 1, 2, 3, 12, 4, 5, 6, 7, 8, 9, 10, 11 };
    size_t s_map[32] = { 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
        38, 39, 40, 41, 42, 43, 44, 45, 46, 47 };
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->if_float)
            p_vreg->reg_id = s_map[p_vreg->reg_id];
        else
            p_vreg->reg_id = r_map[p_vreg->reg_id];
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        if (p_vreg->reg_id != -1) {
            if (p_vreg->if_float)
                p_vreg->reg_id = s_map[p_vreg->reg_id];
            else
                p_vreg->reg_id = r_map[p_vreg->reg_id];
        }
    }
}

static inline void set_save_reg_num(p_symbol_func p_func) {
    p_func->save_reg_r_num = p_func->save_reg_s_num = 0;
    if (p_func->use_reg_num_r > caller_save_reg_num_r)
        p_func->save_reg_r_num = p_func->use_reg_num_r - caller_save_reg_num_r;
    if (p_func->use_reg_num_s > caller_save_reg_num_s)
        p_func->save_reg_s_num = p_func->use_reg_num_s - caller_save_reg_num_s;
}

static inline p_ir_vreg temp_vreg_gen(p_symbol_func p_func) {
    p_ir_vreg p_temp_reg = ir_vreg_gen(symbol_type_var_gen(type_i32));
    p_temp_reg->reg_id = TMP;
    symbol_func_vreg_add(p_func, p_temp_reg);
    return p_temp_reg;
}
static inline bool if_stack_offset_imme(p_ir_operand p_operand) {
    return p_operand->kind == imme && p_operand->p_type->ref_level > 0 && !p_operand->p_vmem->is_global;
}
static inline void offset_reg(p_ir_operand p_operand, p_ir_instr p_instr, p_symbol_func p_func) {
    assert(if_stack_offset_imme(p_operand));
    size_t offset = p_operand->p_vmem->stack_offset + p_operand->offset;
    p_ir_vreg p_temp_reg = temp_vreg_gen(p_func);
    p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_int_gen(offset), p_temp_reg);
    ir_operand_reset_vreg(p_operand, p_temp_reg);
    ir_instr_add_prev(p_assign, p_instr);
}

static inline void deal_unary_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_unary_instr p_unary_instr = &p_instr->ir_unary;
    p_ir_operand p_src = p_unary_instr->p_src;
    switch (p_unary_instr->op) {
    case ir_val_assign:
        if (if_stack_offset_imme(p_src))
            ir_operand_reset_int(p_src, p_src->p_vmem->stack_offset + p_src->offset);
        break;
    case ir_ptr_add_sp:
        if (!if_stack_offset_imme(p_src))
            break;
        size_t offset = p_src->p_vmem->stack_offset + p_src->offset;
        if (!if_legal_rotate_imme12(offset)) {
            offset_reg(p_src, p_instr, p_func);
            break;
        }
        ir_operand_reset_int(p_src, offset);
        break;
    default:
        break;
    }
}

static inline void deal_binary_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_binary_instr p_binary_instr = &p_instr->ir_binary;
    p_ir_vreg p_des = p_binary_instr->p_des;
    p_ir_operand p_src1 = p_binary_instr->p_src1;
    p_ir_operand p_src2 = p_binary_instr->p_src2;
    size_t offset;
    switch (p_binary_instr->op) {
    case ir_add_op:
        if (if_stack_offset_imme(p_src1)) {
            offset = p_src1->p_vmem->stack_offset + p_src1->offset;
            if (p_src2->kind == imme) {
                offset += p_src2->i32const;
                p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_int_gen(offset), p_des);
                ir_instr_add_prev(p_assign, p_instr);
                ir_instr_drop(p_instr);
                break;
            }
            if (!if_legal_rotate_imme12(offset)) {
                offset_reg(p_src1, p_instr, p_func);
                break;
            }
            ir_operand_reset_int(p_src1, offset);
            break;
        }
        if (if_stack_offset_imme(p_src2)) {
            offset = p_src2->p_vmem->stack_offset + p_src2->offset;
            if (p_src1->kind == imme) {
                offset += p_src1->i32const;
                p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_int_gen(offset), p_des);
                ir_instr_add_prev(p_assign, p_instr);
                ir_instr_drop(p_instr);
                break;
            }
            if (!if_legal_rotate_imme12(offset)) {
                offset_reg(p_src2, p_instr, p_func);
                break;
            }
            ir_operand_reset_int(p_src2, offset);
            break;
        }
        break;
    case ir_sub_op:
        assert(!if_stack_offset_imme(p_src2));
        if (if_stack_offset_imme(p_src1)) {
            offset = p_src1->p_vmem->stack_offset + p_src1->offset;
            if (p_src2->kind == imme) {
                offset -= p_src2->i32const;
                p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_int_gen(offset), p_des);
                ir_instr_add_prev(p_assign, p_instr);
                ir_instr_drop(p_instr);
                break;
            }
            if (!if_legal_rotate_imme12(offset)) {
                offset_reg(p_src1, p_instr, p_func);
                break;
            }
            ir_operand_reset_int(p_src1, offset);
            break;
        }
        break;
    default:
        break;
    }
}

static inline void deal_store_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_store_instr p_store_instr = &p_instr->ir_store;
    p_ir_operand p_addr = p_store_instr->p_addr;
    p_ir_vreg p_des = p_store_instr->p_src->p_vreg;
    if (p_store_instr->is_stack_ptr) {
        size_t offset;
        if (if_stack_offset_imme(p_addr))
            offset = p_addr->p_vmem->stack_offset + p_addr->offset;
        else
            return;
        // else can do constant progation rs = 3; str rd, [sp, rs] -> str rd, [sp, 3]
        if (p_des->if_float && !if_legal_imme8_lsl2(offset)) {
            // vstr rd, [sp, #1028]
            // ->
            // tmp = 1028; tmp = sp + tmp; vstr rd, [tmp]
            // now is difficult to do
            // tmp = sp + 1028; vstr rd, [tmp]
            offset_reg(p_addr, p_instr, p_func);
            return;
        }
        if (!if_legal_direct_imme12(offset)) {
            offset_reg(p_addr, p_instr, p_func);
            return;
        }
        ir_operand_reset_int(p_addr, offset);
    }
    // if store a call param in stack, maybe :
    // rd = 3; rd = sp + rd; str rd, []
    // ->
    // rd = sp + 3; str rd, []
    // now is diffcult to do this
}

static inline void deal_load_instr(p_ir_instr p_instr, p_symbol_func p_func) {
    p_ir_load_instr p_load_instr = &p_instr->ir_load;
    p_ir_operand p_addr = p_load_instr->p_addr;
    p_ir_vreg p_des = p_load_instr->p_des;
    if (p_load_instr->is_stack_ptr) {
        size_t offset;
        if (if_stack_offset_imme(p_addr))
            offset = p_addr->p_vmem->stack_offset + p_addr->offset;
        else
            return;
        // else can do constant progation rs = 3; ldr rd, [sp, rs] -> ldr rd, [sp, 3]
        if (p_des->if_float && !if_legal_imme8_lsl2(offset)) {
            // vldr rd, [sp, #1028]
            // ->
            // tmp = 1028; tmp = sp + tmp; vldr rd, [tmp]
            // now is difficult to do
            // tmp = sp + 1028; vldr rd, [tmp]
            offset_reg(p_addr, p_instr, p_func);
            return;
        }
        if (!if_legal_direct_imme12(offset)) {
            offset_reg(p_addr, p_instr, p_func);
            return;
        }
        ir_operand_reset_int(p_addr, offset);
    }
}

static inline void rewrite_after_use_new_vreg(p_ir_instr p_instr, p_ir_vreg p_new_vreg, p_ir_vreg p_origin_vreg) {
    p_list_head p_node, p_node_next;
    list_for_each_safe(p_node, p_node_next, &p_origin_vreg->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        switch (p_use->used_type) {
        case instr_ptr:
            if (p_use->p_instr->instr_id > p_instr->instr_id)
                ir_operand_reset_vreg(p_use, p_new_vreg);
            break;
        case cond_ptr:
        case ret_ptr:
            if (p_use->p_basic_block->block_id >= p_instr->p_basic_block->block_id)
                ir_operand_reset_vreg(p_use, p_new_vreg);
            break;
        case bb_param_ptr:
            if (p_use->p_bb_param->p_target->p_source_block->block_id >= p_instr->p_basic_block->block_id)
                ir_operand_reset_vreg(p_use, p_new_vreg);
            break;
        }
    }
}

static inline void arm_trans_after_func(p_symbol_func p_func) {
    remap_reg_id(p_func);
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if (p_instr->irkind == ir_call) {
                p_list_head p_live_node;
                list_for_each(p_live_node, &p_instr->p_live_out->vreg_list) {
                    p_ir_vreg p_vreg = list_entry(p_live_node, ir_vreg_list_node, node)->p_vreg;
                    if (p_vreg == p_instr->ir_call.p_des)
                        continue;
                    if (!if_caller_save_reg(p_vreg->reg_id))
                        continue;
                    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
                    symbol_func_add_variable(p_func, p_vmem);
                    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem, NULL, 0), ir_operand_vreg_gen(p_vreg), true);
                    ir_instr_add_prev(p_store, p_instr);
                    p_ir_vreg p_new_des = ir_vreg_copy(p_vreg);
                    symbol_func_vreg_add(p_func, p_new_des);
                    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem, NULL, 0), p_new_des, true);
                    ir_instr_add_next(p_load, p_instr);
                    rewrite_after_use_new_vreg(p_instr, p_new_des, p_vreg);
                }
            }
        }
    }
    set_save_reg_num(p_func);
    stack_alloc(p_func);

    list_for_each(p_block_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_binary:
                deal_binary_instr(p_instr, p_func);
            case ir_unary:
                deal_unary_instr(p_instr, p_func);
                break;
            case ir_store:
                deal_store_instr(p_instr, p_func);
                break;
            case ir_load:
                deal_load_instr(p_instr, p_func);
                break;
            default:
                break;
            }
        }
    }
    symbol_func_set_block_id(p_func);
}
void arm_trans_after_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_trans_after_func(p_func);
    }
}
