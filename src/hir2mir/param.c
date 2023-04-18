#include <hir2mir.h>
#include <hir/param.h>
// hir_param_list 转为 mir_param_list
p_mir_param_list hir2mir_param_list_gen(p_hir2mir_info p_info, p_hir_param_list p_h_param_list)
{
    assert(p_h_param_list);
    p_list_head p_node;
    p_mir_param_list p_m_param_list = mir_param_list_init();
    list_for_each(p_node, &p_h_param_list->param){
        p_hir_exp p_exp = list_entry(p_node, hir_param, node)->p_exp;
        p_mir_operand p_op = hir2mir_exp_get_operand(p_info, p_exp);
        mir_param_list_add(p_m_param_list, p_op);
    }
    return p_m_param_list;
}