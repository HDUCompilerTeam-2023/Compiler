#include <hir2mir.h>
#include <symbol/sym.h>
#include <program/gen.h>
#include <program/def.h>

void hir2mir_program_gen(p_program p_program) {
    p_hir2mir_program_info p_program_info = hir2mir_program_info_gen(p_program);
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_hir_func p_h_func = list_entry(p_node, symbol_sym, node)->p_h_func;
        hir2mir_func_gen(p_h_func, p_program_info);
    }
    program_mir_set_vmem_id(p_program);
    hir2mir_program_info_drop(p_program_info);
    program_hir_drop(p_program);
}
