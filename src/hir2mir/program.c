#include <hir2mir.h>
#include <hir/program.h>
#include <hir/symbol_store.h> // 这合适吗 ？
p_mir_program hir2mir_program_gen(p_hir_program p_h_program)
{
    p_mir_program p_m_program = mir_program_gen();
    mir_program_global_set(p_m_program, &p_h_program->pss->global);
    p_hir2mir_info p_info = hir2mir_info_gen();
    p_list_head p_node;
    list_for_each(p_node, &p_h_program->pss->def_function){
        p_symbol_sym p_func_sym = list_entry(p_node, symbol_sym, node);
        p_mir_func p_func = hir2mir_func_gen(p_info, p_func_sym);
        mir_program_func_add(p_m_program,  p_func);
    }
    p_m_program->p_operand_list = p_info->p_operand_list;
    hir2mir_info_drop(p_info);
    return p_m_program;
}