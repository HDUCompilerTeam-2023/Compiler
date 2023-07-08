#include <ast/block.h>
#include <ast/stmt.h>
#include <ast2ir.h>

#include <symbol/type.h>
#include <symbol/var.h>

void ast2ir_stmt_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_cond, p_ir_basic_block p_while_end_next, p_ast_stmt p_stmt) {
    if (!p_stmt) return;
    switch (p_stmt->type) {
    case ast_stmt_assign:
        ast2ir_stmt_assign_gen(p_info, p_stmt->p_lval, p_stmt->p_rval, p_stmt->is_stack);
        break;
    case ast_stmt_return:
        ast2ir_stmt_return_gen(p_info, p_stmt->p_exp);
        break;
    case ast_stmt_exp:
        ast2ir_stmt_exp_gen(p_info, p_stmt->p_exp);
        break;
    case ast_stmt_block:
        ast2ir_stmt_block_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_block);
        break;
    case ast_stmt_if:
        ast2ir_stmt_if_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1);
        break;
    case ast_stmt_if_else:
        ast2ir_stmt_if_else_gen(p_info, p_while_cond, p_while_end_next, p_stmt->p_exp, p_stmt->p_stmt_1, p_stmt->p_stmt_2);
        break;
    case ast_stmt_while:
        ast2ir_stmt_while_gen(p_info, p_stmt->p_exp, p_stmt->p_stmt_1);
        break;
    case ast_stmt_break:
        ast2ir_stmt_break_gen(p_info, p_while_end_next);
        break;
    case ast_stmt_continue:
        ast2ir_stmt_continue_gen(p_info, p_while_cond);
        break;
    }
}

void ast2ir_stmt_assign_gen(p_ast2ir_info p_info, p_ast_exp p_lval, p_ast_exp p_rval, bool is_stack) {
    p_ir_operand p_src = ast2ir_exp_gen(p_info, p_rval);
    p_ir_operand p_addr = ast2ir_exp_gen(p_info, p_lval);
    p_ir_instr p_instr = ir_store_instr_gen(p_addr, p_src, is_stack);
    ast2ir_info_add_instr(p_info, p_instr);
}
// 将返回值全部放到 一个变量, 并跳转到 ret 块
void ast2ir_stmt_return_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    if (p_exp) {
        assert(p_info->p_ret_vmem);
        p_ir_operand p_ret = ast2ir_exp_gen(p_info, p_exp);
        p_ir_operand p_ret_addr_operand = ir_operand_addr_gen(p_info->p_ret_vmem);
        ast2ir_info_add_instr(p_info, ir_store_instr_gen(p_ret_addr_operand, p_ret, true));
    }
    else {
        assert(!p_info->p_ret_vmem);
    }
    ir_basic_block_set_br(p_info->p_current_basic_block, p_info->p_ret_block);
    p_ir_basic_block p_next = ir_basic_block_gen();
    ast2ir_info_add_basic_block(p_info, p_next);
    return;
}
void ast2ir_stmt_exp_gen(p_ast2ir_info p_info, p_ast_exp p_exp) {
    if (!p_exp) return;
    p_ir_operand p_operand = ast2ir_exp_gen(p_info, p_exp);
    if (p_operand) ir_operand_drop(p_operand);
}
// 跳转到循环体外并新建一个基本块作为之后指令写入的基本块
void ast2ir_stmt_break_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_end_next) {
    ir_basic_block_set_br(p_info->p_current_basic_block, p_while_end_next);
    p_ir_basic_block p_next = ir_basic_block_gen();
    ast2ir_info_add_basic_block(p_info, p_next);
    return;
}

void ast2ir_stmt_continue_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_cond) {
    ir_basic_block_set_br(p_info->p_current_basic_block, p_while_cond);
    p_ir_basic_block p_next = ir_basic_block_gen();
    ast2ir_info_add_basic_block(p_info, p_next);
    return;
}

void ast2ir_stmt_if_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_cond, p_ir_basic_block p_while_end_next, p_ast_exp p_exp, p_ast_stmt p_stmt_1) {

    p_ir_basic_block p_true_block = ir_basic_block_gen();
    p_ir_basic_block p_next_block = ir_basic_block_gen();

    // 解析条件表达式
    ast2ir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 解析 true 的 block
    ast2ir_info_add_basic_block(p_info, p_true_block);
    ast2ir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_1);

    // true block 的末尾添加跳转
    ir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 重新置当前写 block 为 p_next_block
    ast2ir_info_add_basic_block(p_info, p_next_block);
    return;
}

void ast2ir_stmt_if_else_gen(p_ast2ir_info p_info, p_ir_basic_block p_while_cond, p_ir_basic_block p_while_end_next, p_ast_exp p_exp, p_ast_stmt p_stmt_1, p_ast_stmt p_stmt_2) {

    p_ir_basic_block p_true_block = ir_basic_block_gen();
    p_ir_basic_block p_false_block = ir_basic_block_gen();
    p_ir_basic_block p_next_block = ir_basic_block_gen();

    // 生成条件表达式
    ast2ir_exp_cond_gen(p_info, p_true_block, p_false_block, p_exp);

    // 生成 true 情况下的语句
    ast2ir_info_add_basic_block(p_info, p_true_block);
    ast2ir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_1);

    // 在 true_block 末尾添加跳转
    ir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 生成 false 情况下的语句
    ast2ir_info_add_basic_block(p_info, p_false_block);
    ast2ir_stmt_gen(p_info, p_while_cond, p_while_end_next, p_stmt_2);

    // false 的末尾block 添加跳转
    ir_basic_block_set_br(p_info->p_current_basic_block, p_next_block);

    // 重新置当前写 block 为 p_next_block
    ast2ir_info_add_basic_block(p_info, p_next_block);
    return;
}

void ast2ir_stmt_while_gen(p_ast2ir_info p_info, p_ast_exp p_exp, p_ast_stmt p_stmt_1) {
    // 转换成  do while

    p_ir_basic_block p_true_block = ir_basic_block_gen();
    p_ir_basic_block p_next_block = ir_basic_block_gen();
    p_ir_basic_block p_inner_cond_block = ir_basic_block_gen();

    // 当前块添加跳转
    ir_basic_block_set_br(p_info->p_current_basic_block, p_inner_cond_block);

    // 条件跳转块
    ast2ir_info_add_basic_block(p_info, p_inner_cond_block);
    ast2ir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);
    // 解析 while 循环体
    ast2ir_info_add_basic_block(p_info, p_true_block);
    ast2ir_stmt_gen(p_info, p_inner_cond_block, p_next_block, p_stmt_1);
    ast2ir_exp_cond_gen(p_info, p_true_block, p_next_block, p_exp);

    // 置当前写的块为下一块
    ast2ir_info_add_basic_block(p_info, p_next_block);

    return;
}
void ast2ir_stmt_block_gen(p_ast2ir_info p_info, p_ir_basic_block while_start, p_ir_basic_block while_end_next, p_ast_block p_block) {
    p_list_head p_node;
    list_for_each(p_node, &p_block->stmt) {
        p_ast_stmt p_stmt = list_entry(p_node, ast_stmt, node);
        ast2ir_stmt_gen(p_info, while_start, while_end_next, p_stmt);
    }
    return;
}
