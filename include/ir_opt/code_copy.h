#ifndef __IR_OPT_CODE_COPY__
#define __IR_OPT_CODE_COPY__

#include <ir.h>
#include <symbol.h>
#include <program/use.h>

typedef struct copy_map copy_map, *p_copy_map;

p_copy_map ir_code_copy_map_gen(void);
void ir_code_copy_map_drop(p_copy_map p_map);

p_ir_vreg ir_code_copy_vreg(p_ir_vreg p_vreg, p_copy_map p_map);
p_symbol_var ir_code_copy_var(p_symbol_var p_var, p_copy_map p_map);
p_ir_basic_block ir_code_copy_bb(p_ir_basic_block p_bb, p_copy_map p_map);
void ir_code_copy_instr_of_block(p_ir_basic_block p_src, p_copy_map p_map);
void loop_block_vreg_copy(p_ir_basic_block p_block, p_copy_map p_map);
void ir_code_copy_instr_of_block_inline(p_ir_basic_block p_src, p_copy_map p_map, p_ir_instr p_call, p_ir_basic_block p_next);

#endif
