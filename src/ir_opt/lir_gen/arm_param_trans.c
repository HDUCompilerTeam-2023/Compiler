#include <ir_gen.h>
#include <ir_opt/lir_gen/arm_standard.h>
#include <program/def.h>
#include <symbol_gen.h>
static inline void new_load_func_param(p_symbol_func p_func, p_ir_basic_block p_entry, p_ir_vreg p_param) {
    p_symbol_var p_param_vmem = symbol_func_param_reg_mem(p_func, p_param);
    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_param_vmem, NULL, 0), p_param, true);
    ir_basic_block_addinstr_head(p_entry, p_load);
}

static inline void symbol_func_param_vreg2vmem(p_symbol_func p_func) {
    p_ir_basic_block p_entry = p_func->p_entry_block;
    size_t r = 0;
    size_t s = 0;
    p_list_head p_node_vreg, p_node_vreg_next;
    list_for_each_safe(p_node_vreg, p_node_vreg_next, &p_func->param_reg_list) {
        p_ir_vreg p_param = list_entry(p_node_vreg, ir_vreg, node);
        if (if_in_r(p_param->p_type)) {
            if (r >= temp_reg_num_r)
                new_load_func_param(p_func, p_entry, p_param);
            r++;
        }
        else {
            if (s >= temp_reg_num_s)
                new_load_func_param(p_func, p_entry, p_param);
            s++;
        }
    }
}
static inline void arm_param_trans_func(p_symbol_func p_func){
    symbol_func_param_vreg2vmem(p_func);
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_call_graph_node->callee){
        p_call_graph_edge p_edge = list_entry(p_node, call_graph_edge, caller_node);
        p_list_head p_node;
        list_for_each(p_node, &p_edge->call_instr){
            p_ir_instr p_instr = list_entry(p_node, call_instr_node, node)->p_call_instr;
            assert(p_instr->irkind == ir_call);
            size_t r = 0;
            size_t s = 0;
            int offset = 0;
            p_list_head p_node;
            list_for_each(p_node, &p_instr->ir_call.param_list) {
                p_ir_param p_param = list_entry(p_node, ir_param, node);
                if (if_in_r(p_param->p_param->p_type)) {
                    if (r >= temp_reg_num_r)
                        p_param->is_in_mem = true;
                    r++;
                }
                else {
                    // if (p_param->p_param->kind == reg)
                    //     assert(p_param->p_param->p_vreg->if_float);
                    if (s >= temp_reg_num_s)
                        p_param->is_in_mem = true;
                    s++;
                }
                if (p_param->is_in_mem) {
                    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_param->p_param->p_type));
                    offset -= basic_type_get_size(p_vmem->p_type->basic);
                    p_vmem->stack_offset = offset;
                    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem, NULL, 0), ir_operand_copy(p_param->p_param), true);
                    ir_instr_add_prev(p_store, p_instr);
                    ir_param_set_vmem(p_param, p_vmem);
                }
            }
        }
    }
}
void arm_param_trans_pass(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function){
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        arm_param_trans_func(p_func);
    }
}
