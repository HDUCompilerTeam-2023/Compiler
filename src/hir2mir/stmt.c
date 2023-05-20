#include <hir/block.h>
#include <hir/stmt.h>
#include <hir2mir.h>

#include <symbol/sym.h>
#include <symbol/type.h>

void hir2mir_stmt_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_cond, p_mir_basic_block p_while_end_next, p_hir_stmt p_stmt) {
    if (!p_stmt) return;
    switch (p_stmt->type) {
    case hir_stmt_return:
        hir2mir_stmt_return_gen(p_info, p_stmt->p_exp);
        break;
    case hir_stmt_exp:
        hir2mir_stmt_exp_gen(p_info, p_stmt->p_exp);
        break;
    case hir_stmt_block:
        hir2mir_stmt_block_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_block);
        break;
    case hir_stmt_if:
        hir2mir_stmt_if_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1);
        break;
    case hir_stmt_if_else:
        hir2mir_stmt_if_else_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1, p_stmt->p_stmt_2);
        break;
    case hir_stmt_while:
        hir2mir_stmt_while_gen(p_info, p_stmt->p_exp, p_stmt->p_stmt_1);
        break;
    case hir_stmt_break:
        hir2mir_stmt_break_gen(p_info, p_while_end_next);
        break;
    case hir_stmt_continue:
        hir2mir_stmt_continue_gen(p_info, p_while_cond);
        break;
    }
}

// 将返回值全部放到 一个变量, 并跳转到 ret 块
void hir2mir_stmt_return_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    p_mir_operand p_ret = NULL;
    if (p_exp)
        p_ret = hir2mir_exp_gen(p_info, p_exp);
    else
        p_ret = mir_operand_void_gen();
    p_mir_operand p_ret_addr_operand = mir_operand_vreg_gen(p_info->p_ret_addr);
    hir2mir_info_add_instr(p_info, mir_store_instr_gen(p_ret_addr_operand, NULL, p_ret));

    mir_basic_block_set_br(p_info->p_current_basic_block, p_info->p_ret_block);
    p_mir_basic_block p_next = mir_basic_block_gen();
    hir2mir_info_add_basic_block(p_info, p_next);
    return;
}
void hir2mir_stmt_exp_gen(p_hir2mir_info p_info, p_hir_exp p_exp) {
    if (!p_exp) return;
    p_mir_operand p_operand = hir2mir_exp_gen(p_info, p_exp);
    if (p_operand) mir_operand_drop(p_operand);
}
// 跳转到循环体外并新建一个基本块作为之后指令写入的基本块
void hir2mir_stmt_break_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_end_next) {
    mir_basic_block_set_br(p_info->p_current_basic_block, p_while_end_next);
    p_mir_basic_block p_next = mir_basic_block_gen();
    hir2mir_info_add_basic_block(p_info, p_next);
    return;
}

void hir2mir_stmt_continue_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_cond) {
    mir_basic_block_set_br(p_info->p_current_basic_block, p_while_cond);
    p_mir_basic_block p_next = mir_basic_block_gen();
    hir2mir_info_add_basic_block(p_info, p_next);
    return;
}

void hir2mir_stmt_if_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_cond, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1) {

    p_mir_basic_block p_true_block = mir_basic_block_gen();
    p_mir_basic_block p_next_block = mir_basic_block_gen();

    // 解析条件表达式
    hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 解析 true 的 block
    hir2mir_info_add_basic_block(p_info, p_true_block);
    hir2mir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_1);

    // true block 的末尾添加跳转
    mir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 重新置当前写 block 为 p_next_block
    hir2mir_info_add_basic_block(p_info, p_next_block);
    return;
}

void hir2mir_stmt_if_else_gen(p_hir2mir_info p_info, p_mir_basic_block p_while_cond, p_mir_basic_block p_while_end_next, p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2) {

    p_mir_basic_block p_true_block = mir_basic_block_gen();
    p_mir_basic_block p_false_block = mir_basic_block_gen();
    p_mir_basic_block p_next_block = mir_basic_block_gen();

    // 生成条件表达式
    hir2mir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp);

    // 生成 true 情况下的语句
    hir2mir_info_add_basic_block(p_info, p_true_block);
    hir2mir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_1);

    // 在 true_block 末尾添加跳转
    mir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 生成 false 情况下的语句
    hir2mir_info_add_basic_block(p_info, p_false_block);
    hir2mir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_2);

    // false 的末尾block 添加跳转
    mir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 重新置当前写 block 为 p_next_block
    hir2mir_info_add_basic_block(p_info, p_next_block);
    return;
}

void hir2mir_stmt_while_gen(p_hir2mir_info p_info, p_hir_exp p_exp, p_hir_stmt p_stmt_1) {
    // 转换成  do while

    p_mir_basic_block p_true_block = mir_basic_block_gen();
    p_mir_basic_block p_next_block = mir_basic_block_gen();
    p_mir_basic_block p_inner_cond_block = mir_basic_block_gen();

    // 解析条件表达式， 在当前块已经生成 条件跳转
    hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 条件跳转块
    hir2mir_info_add_basic_block(p_info, p_inner_cond_block);
    hir2mir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);
    // 解析 while 循环体
    hir2mir_info_add_basic_block(p_info, p_true_block);
    hir2mir_stmt_gen(p_info, p_inner_cond_block, p_next_block, p_stmt_1);
    mir_basic_block_set_br(p_info->p_current_basic_block, p_inner_cond_block);

    // 置当前写的块为下一块
    hir2mir_info_add_basic_block(p_info, p_next_block);

    return;
}
void hir2mir_stmt_block_gen(p_hir2mir_info p_info, p_mir_basic_block while_start, p_mir_basic_block while_end_next, p_hir_block p_block) {
    p_list_head p_node;
    list_for_each(p_node, &p_block->stmt) {
        p_hir_stmt p_stmt = list_entry(p_node, hir_stmt, node);
        hir2mir_stmt_gen(p_info, while_start, while_end_next, p_stmt);
    }
    return;
}
