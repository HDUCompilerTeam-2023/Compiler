#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/func.h>
#include <ir_manager/set_cond.h>
#include <ir/basic_block.h>
#include <ir_gen/instr.h>
#include <ir/vreg.h>
#include <ir_gen/operand.h>

void ir_opt_copy_propagation(p_program p_ir) {
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        p_list_head p_node;
        list_for_each(p_node, &p_func->block) {
            p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_bb->instr_list) {
                p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
                if (p_instr->irkind != ir_unary)
                    continue;
                if (p_instr->ir_unary.op != ir_val_assign)
                    continue;
                p_ir_operand p_src_op = p_instr->ir_unary.p_src;
                if (p_src_op->kind != reg)
                    continue;
                assert(p_src_op->p_vreg);
                p_ir_vreg p_src = p_src_op->p_vreg;
                p_ir_vreg p_des = p_instr->ir_unary.p_des;
                p_list_head p_node, p_next;
                list_for_each_safe(p_node, p_next, &p_des->use_list) {
                    p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
                    assert(p_use->kind == reg);
                    ir_operand_reset_vreg(p_use, p_src);
                }
                ir_instr_drop(p_instr);
                symbol_func_vreg_del(p_func, p_des);
            }
        }
    }
    ir_cfg_set_program_dom(p_ir);
    ir_build_program_nestedtree(p_ir);
    set_cond_pass(p_ir);
}

