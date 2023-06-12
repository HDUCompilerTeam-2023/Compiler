#include <ast2ir.h>
#include <program/def.h>
#include <symbol/type.h>
#include <symbol_gen/func.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_ast2ir_info ast2ir_info_gen(p_symbol_func p_m_func, p_program p_program) {
    p_ast2ir_info p_info = malloc(sizeof(*p_info));
    *p_info = (ast2ir_info) {
        .p_current_basic_block = NULL,
        .p_func = p_m_func,
        .p_program = p_program,
        .p_ret_vmem = NULL,
        .p_ret_block = ir_basic_block_gen(),
    };

    p_ir_basic_block p_entry = ir_basic_block_gen();
    ast2ir_info_add_basic_block(p_info, p_entry);

    return p_info;
}

void ast2ir_info_drop(p_ast2ir_info p_info) {
    free(p_info);
}

void ast2ir_info_add_basic_block(p_ast2ir_info p_info, p_ir_basic_block p_new) {
    symbol_func_bb_add(p_info->p_func, p_new);
    p_info->p_current_basic_block = p_new;
}

void ast2ir_info_add_instr(p_ast2ir_info p_info, p_ir_instr p_instr) {
    ir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
    p_ir_vreg p_des = ir_instr_get_des(p_instr);
    if (p_des) {
        symbol_func_vreg_add(p_info->p_func, p_des);
    }
}
