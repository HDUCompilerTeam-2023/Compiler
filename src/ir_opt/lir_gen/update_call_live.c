#include <ir_opt/lir_gen/update_call_live.h>
#include <ir_gen.h>
#include <program/def.h>
#include <symbol_gen.h>

static void update_call_func_live(p_symbol_func p_func){
    p_list_head p_block_node;
    list_for_each(p_block_node, &p_func->block){
        p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
        p_list_head p_instr_node, p_instr_node_next;
        list_for_each_safe(p_instr_node, p_instr_node_next, &p_basic_block->instr_list){
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            if(p_instr->irkind == ir_call){
                p_list_head p_live_node;
                list_for_each(p_live_node, &p_instr->p_live_out->bb_phi){
                    p_ir_vreg p_vreg = list_entry(p_live_node, ir_bb_phi, node)->p_bb_phi;
                    if(p_vreg == p_instr->ir_call.p_des)
                        continue;
                    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
                    symbol_func_add_variable(p_func, p_vmem);
                    p_ir_instr p_store = ir_store_instr_gen(ir_operand_addr_gen(p_vmem), NULL, ir_operand_vreg_gen(p_vreg));
                    list_add_prev(&p_store->node, &p_instr->node);
                    p_ir_instr p_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem), NULL, p_vreg);
                    list_add_next(&p_load->node, &p_instr->node);
                }
            }
        }
    }
}
void update_call_live_pass(p_program p_ir)
{
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function){
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        update_call_func_live(p_func);
    }
}