#include <ast/param.h>
#include <ast2ir.h>
// ast_param_list 转为 ir_param_list
p_ir_param_list ast2ir_param_list_gen(p_ast2ir_info p_info, p_ast_param_list p_h_param_list) {
    assert(p_h_param_list);
    p_list_head p_node;
    p_ir_param_list p_m_param_list = ir_param_list_init();
    list_for_each(p_node, &p_h_param_list->param) {
        p_ast_exp p_exp = list_entry(p_node, ast_param, node)->p_exp;
        p_ir_operand p_op = ast2ir_exp_gen(p_info, p_exp);
        ir_param_list_add(p_m_param_list, p_op);
    }
    return p_m_param_list;
}
