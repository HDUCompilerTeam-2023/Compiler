#include <ir/basic_block.h>
#include <ir/operand.h>
#include <ir/vreg.h>
#include <ir_opt/lir_gen/set_cond.h>
#include <program/def.h>
#include <symbol/func.h>

void set_cond_pass(p_program p_ir) {
    p_list_head p_func_node;
    list_for_each(p_func_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_func_node, symbol_func, node);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_block_node, ir_basic_block, node);
            if (p_basic_block->p_branch->kind == ir_cond_branch) {
                assert(p_basic_block->p_branch->p_exp->kind == reg);
                p_ir_vreg p_cond = p_basic_block->p_branch->p_exp->p_vreg;
                p_cond->if_cond = true;
            }
        }
    }
}