#include <hir2mir.h>
#include <hir_gen.h>
#include <symbol/store.h>
#include <symbol/sym.h> // 这合适吗 ？

#include <hir/func.h>
p_mir_program hir2mir_program_gen(p_hir_program p_h_program) {
    p_mir_program p_m_program = mir_program_gen(list_entry(p_h_program->func.p_prev, hir_func, node)->p_sym->id + 1);
    p_m_program->p_store = p_h_program->p_store;
    p_hir2mir_program_info p_program_info = hir2mir_program_info_gen(p_m_program);
    p_list_head p_node;
    list_for_each(p_node, &p_h_program->func) {
        p_hir_func p_h_func = list_entry(p_node, hir_func, node);
        hir2mir_func_gen(p_h_func, p_program_info);
    }
    mir_program_set_vmem_id(p_m_program);
    hir2mir_program_info_drop(p_program_info);
    hir_program_drop(p_h_program);
    return p_m_program;
}
