#ifndef __AST2IR_INFO_GEN__
#define __AST2IR_INFO_GEN__

#include <ast2ir/info.h>

#include <program/use.h>

struct ast2ir_info {
    p_ir_basic_block p_current_basic_block;
    p_symbol_func p_func;
    p_program p_program;

    p_ir_basic_block p_ret_block; // 返回的块
    p_symbol_var p_ret_vmem; // 返回值所存储的位置
};

p_ast2ir_info ast2ir_info_gen(p_symbol_func p_m_func, p_program p_program);

void ast2ir_info_add_instr(p_ast2ir_info p_info, p_ir_instr p_instr);
void ast2ir_info_add_basic_block(p_ast2ir_info p_info, p_ir_basic_block p_basic_block);

void ast2ir_info_drop(p_ast2ir_info p_info);
#include <ir_gen.h>
#endif
