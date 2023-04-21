#include <hir2mir.h>
#include <hir/block.h>
#include <hir/stmt.h>

#include <symbol/sym.h>
#include <symbol/type.h>

p_mir_instr hir2mir_stmt_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start, p_mir_basic_block p_while_end_next, p_hir_stmt p_stmt)
{
    switch (p_stmt->type) {
        case hir_stmt_init:
            return hir2mir_stmt_init_gen(p_info, p_stmt->p_sym);
        case hir_stmt_return:
            return hir2mir_stmt_return_gen(p_info, p_stmt->p_exp);
        case hir_stmt_exp:
            return hir2mir_stmt_exp_gen(p_info, p_stmt->p_exp);
        case hir_stmt_block:
            return hir2mir_stmt_block_gen(p_info, p_while_start, p_while_end_next, p_stmt->p_block);
        case hir_stmt_if:
            return hir2mir_stmt_if_gen(p_info, p_while_start, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1);
        case hir_stmt_if_else:
            return hir2mir_stmt_if_else_gen(p_info, p_while_start, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1, p_stmt->p_stmt_2);
        case hir_stmt_while:
            return hir2mir_stmt_while_gen(p_info, p_stmt->p_exp, p_stmt->p_stmt_1);
        case hir_stmt_break:
            return hir2mir_stmt_break_gen(p_info, p_while_end_next);
        case hir_stmt_continue:
            return hir2mir_stmt_break_gen(p_info, p_while_start);
    }
}
p_mir_instr hir2mir_stmt_init_gen(p_hir2mir_info p_info, p_symbol_sym p_sym)
{
    assert(p_sym && !p_sym->is_global && p_sym->p_init);
    p_mir_operand p_des_sym = hir2mir_operand_declared_sym_gen(p_info, p_sym);
    p_mir_operand p_src = NULL;
    p_mir_instr p_new_instr = NULL;
    if (p_sym->p_type->kind == type_var) {
        p_src = hir2mir_exp_get_operand(p_info, p_sym->p_init->memory[0]);
        p_new_instr = mir_unary_instr_gen(mir_val_assign, p_src, p_des_sym);
        mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
    }
    else if (p_sym->p_type->kind == type_arrary){
        for (size_t i = 0; i < p_sym->p_init->size; i ++) {
            p_src = hir2mir_exp_get_operand(p_info, p_sym->p_init->memory[i]);
            p_mir_operand p_offset = hir2mir_operand_int_gen(p_info, i);
            p_new_instr = mir_array_assign_instr_gen(p_des_sym, p_offset, p_src);
            mir_basic_block_addinstr(p_info->p_current_basic_block, p_new_instr);
        }
    }
    return p_new_instr;
}

// 将返回值全部放到 一个变量, 并跳转到 ret 块
p_mir_instr hir2mir_stmt_return_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{   
    p_mir_operand p_ret = hir2mir_operand_void_gen(p_info);
    if (p_exp) 
        p_ret = hir2mir_exp_get_operand(p_info, p_exp);
    p_mir_instr p_ret_assign_instr = mir_unary_instr_gen(mir_val_assign, p_ret, p_info->p_ret_operand);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_ret_assign_instr);
    p_mir_instr p_br_ret_instr = mir_br_instr_gen(p_info->p_current_basic_block, p_info->p_ret_block);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_br_ret_instr);
    p_info->p_current_basic_block = hir2mir_basic_block_gen(p_info);
    return p_br_ret_instr;
}
p_mir_instr hir2mir_stmt_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp)
{
    return hir2mir_exp_gen(p_info, p_exp);
}
// 跳转到循环体外并新建一个基本块作为之后指令写入的基本块
p_mir_instr hir2mir_stmt_break_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_end_next)
{
    p_mir_instr p_br = mir_br_instr_gen(p_info->p_current_basic_block, p_while_end_next);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_br);
    p_info->p_current_basic_block = hir2mir_basic_block_gen(p_info);
    return p_br;
}

p_mir_instr hir2mir_stmt_continue_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start)
{
    p_mir_instr p_br = mir_br_instr_gen(p_info->p_current_basic_block, p_while_start);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_br);
    p_info->p_current_basic_block = hir2mir_basic_block_gen(p_info);
    return p_br;
}

p_mir_instr hir2mir_stmt_if_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1)
{    

    p_mir_basic_block p_true_block = hir2mir_basic_block_gen(p_info);
    p_mir_basic_block p_next_block = hir2mir_basic_block_gen(p_info);
    
    // 解析条件表达式
    p_mir_instr p_new_instr = hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 解析 true 的 block
    p_info->p_current_basic_block = p_true_block;
    hir2mir_stmt_gen(p_info, p_while_start, p_while_end_next, p_stmt_1);

    // true block 的末尾添加跳转
    p_mir_instr p_true_block_br = mir_br_instr_gen(p_info->p_current_basic_block, p_next_block);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_true_block_br);

    // 重新置当前写 block 为 p_next_block
    p_info->p_current_basic_block = p_next_block;
    return p_new_instr;
}

p_mir_instr hir2mir_stmt_if_else_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_start, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2)
{
    
    p_mir_basic_block p_true_block = hir2mir_basic_block_gen(p_info);
    p_mir_basic_block p_false_block = hir2mir_basic_block_gen(p_info);
    p_mir_basic_block p_next_block = hir2mir_basic_block_gen(p_info);

    // 生成条件表达式
    p_mir_instr p_new_instr = hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp);

    // 生成 true 情况下的语句
    p_info->p_current_basic_block = p_true_block;
    hir2mir_stmt_gen(p_info, p_while_start, p_while_end_next, p_stmt_1);

    // 在 true_block 末尾添加跳转
    p_mir_instr p_true_block_br = mir_br_instr_gen(p_info->p_current_basic_block, p_next_block);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_true_block_br);

    // 生成 false 情况下的语句
    p_info->p_current_basic_block = p_false_block;
    hir2mir_stmt_gen(p_info, p_while_start, p_while_end_next, p_stmt_2);

    // false 的末尾block 添加跳转
    p_mir_instr p_false_block_br = mir_br_instr_gen(p_info->p_current_basic_block, p_next_block);
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_false_block_br);

    // 重新置当前写 block 为 p_next_block
    p_info->p_current_basic_block = p_next_block;
    return p_new_instr;
}

p_mir_instr hir2mir_stmt_while_gen(p_hir2mir_info p_info, p_hir_exp p_exp, p_hir_stmt p_stmt_1)
{
    // 转换成  do while


    p_mir_basic_block p_true_block = hir2mir_basic_block_gen(p_info);
    p_mir_basic_block p_next_block = hir2mir_basic_block_gen(p_info);

    // 解析条件表达式， 在当前块已经生成 条件跳转
    p_mir_instr p_condbr_outwhile = hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 解析 while 循环体
    p_info->p_current_basic_block = p_true_block;
    hir2mir_stmt_gen(p_info, p_true_block, p_next_block, p_stmt_1);
    hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);
    // 条件语句跳转已经生成完成，不用添加

    // 置当前写的块为下一块
    p_info->p_current_basic_block = p_next_block;
    
    return p_condbr_outwhile;
}
p_mir_instr hir2mir_stmt_block_gen(p_hir2mir_info p_info, p_mir_basic_block while_start, p_mir_basic_block while_end_next, p_hir_block p_block)
{
    p_list_head p_node;
    p_mir_instr p_last_instr = NULL;
    list_for_each(p_node, &p_block->stmt){
        p_hir_stmt p_stmt = list_entry(p_node, hir_stmt, node);
        p_last_instr = hir2mir_stmt_gen(p_info, while_start, while_end_next, p_stmt);
    }
    return  p_last_instr;
}