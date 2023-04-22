#include <hir2mir.h>
#include <hir/program.h>
#include <symbol/sym.h> // 这合适吗 ？
#include <symbol/store.h>

#include <hir/func.h>
p_mir_program hir2mir_program_gen(p_hir_program p_h_program)
{
    p_mir_program p_m_program = mir_program_gen();
    mir_program_global_set(p_m_program, &p_h_program->p_store->variable);
    p_list_head p_node;
    list_for_each(p_node, &p_h_program->func){
        p_hir_func p_h_func = list_entry(p_node, hir_func, node);
        if (p_h_func->p_block) {
            p_mir_func p_m_func = hir2mir_func_gen(p_h_func);
            mir_program_func_add(p_m_program,  p_m_func);
        }
    }
    return p_m_program;
}