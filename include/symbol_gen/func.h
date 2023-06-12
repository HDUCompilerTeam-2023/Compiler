#ifndef __SYMBOL_GEN_FUNC__
#define __SYMBOL_GEN_FUNC__

#include <symbol/func.h>

p_symbol_func symbol_func_gen(const char *name, basic_type b_type, bool is_va);

void symbol_func_add_constant(p_symbol_func p_func, p_symbol_var p_var);
void symbol_func_add_variable(p_symbol_func p_func, p_symbol_var p_var);
void symbol_func_add_param(p_symbol_func p_func, p_symbol_var p_var);

void symbol_func_drop(p_symbol_func p_func);

void symbol_func_bb_add(p_symbol_func p_func, p_ir_basic_block p_basic_block);
void symbol_func_bb_del(p_symbol_func p_func, p_ir_basic_block p_basic_block);
void symbol_func_param_reg_add(p_symbol_func p_func, p_ir_vreg p_vreg);
void symbol_func_param_reg_del(p_symbol_func p_func, p_ir_vreg p_vreg);
void symbol_func_vreg_add(p_symbol_func p_func, p_ir_vreg p_vreg);
void symbol_func_vreg_del(p_symbol_func p_func, p_ir_vreg p_vreg);
void symbol_func_vreg_add_at(p_symbol_func p_func, p_ir_vreg p_new_sym, p_ir_basic_block p_current_block, p_ir_instr p_instr);
void symbol_func_basic_block_init_visited(p_symbol_func p_func);
void symbol_func_set_block_id(p_symbol_func p_func);
void symbol_func_set_vreg_id(p_symbol_func p_func);

#endif
