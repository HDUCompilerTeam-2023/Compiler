#include <ir_gen.h>
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
    symbol_func_clear_varible(p_func);
    size_t stack_size = p_func->inner_stack_size;
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_vmem->stack_offset = stack_size;
        stack_size += p_vmem->p_type->size;
    }
    list_for_each(p_node, &p_func->constant) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_vmem->stack_offset = stack_size;
        stack_size += p_vmem->p_type->size;
    }
    p_func->stack_size = alignTo(stack_size + 1, 2) - 1;
    stack_size = p_func->stack_size + 1;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        p_vmem->stack_offset = stack_size++;
    }
}

static inline void remap_reg_id(p_symbol_func p_func) {
    size_t r_map[13] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
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
                list_for_each(p_live_node, &p_instr->p_live_out->bb_phi) {
                    p_ir_vreg p_vreg = list_entry(p_live_node, ir_bb_phi, node)->p_bb_phi;
                    if (p_vreg == p_instr->ir_call.p_des)
                        continue;
                    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
                    symbol_func_add_variable(p_func, p_vmem);
                    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem), ir_operand_vreg_gen(p_vreg), true);
                    list_add_prev(&p_store->node, &p_instr->node);
                    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem), p_vreg, true);
                    list_add_next(&p_load->node, &p_instr->node);
                }
            }
        }
    }
    stack_alloc(p_func);
}
void arm_trans_after_pass(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_trans_after_func(p_func);
    }
}
