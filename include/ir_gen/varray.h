#ifndef __IR_GEN_VARRAY__
#define __IR_GEN_VARRAY__

#include <ir/varray.h>
p_ir_vmem_base ir_vmem_base_vmem_gen(p_symbol_var p_var);
p_ir_vmem_base ir_vmem_base_param_gen(p_ir_param_vmem_base p_param);
p_ir_param_vmem_base ir_param_vmem_base_gen(p_ir_vreg p_vreg, p_symbol_func p_func);

p_ir_varray ir_varray_gen(p_ir_vmem_base p_base);
p_ir_varray ir_varray_copy(p_ir_varray p_src);
p_ir_varray_use ir_varray_use_gen(p_ir_varray p_varray);
void ir_varray_use_reset_varray(p_ir_varray_use p_use, p_ir_varray p_varray);
p_ir_varray_def_pair ir_varray_def_pair_gen(p_ir_varray p_des, p_ir_varray_use p_src);
void ir_varray_set_instr_def(p_ir_varray p_varray, p_ir_instr p_instr);
void ir_varray_set_func_def(p_ir_varray p_varray, p_symbol_func p_func);
void ir_varray_set_bb_phi_def(p_ir_varray p_varray, p_ir_varray_bb_phi p_varray_bb_phi);
void ir_varray_set_global_def(p_ir_varray p_varray);
void ir_varray_set_instr_use(p_ir_varray_use p_varray_use, p_ir_instr p_instr);
void ir_varray_set_bb_parram_use(p_ir_varray_use p_varray_use, p_ir_varray_bb_param p_ir_varray_bb_param);

void ir_vmem_base_set_varray_id(p_ir_vmem_base p_base);

void ir_varray_use_drop(p_ir_varray_use p_use);
void ir_vmem_base_clear(p_ir_vmem_base p_base);
void ir_vmem_base_clear_all(p_ir_vmem_base p_base);
void ir_vmem_base_drop(p_ir_vmem_base p_base);
void ir_varray_drop(p_ir_varray p_varray);
void ir_varray_def_pair_drop(p_ir_varray_def_pair p_def_pair);
void ir_param_vmem_base_drop(p_ir_param_vmem_base p_vmem_base);
#endif