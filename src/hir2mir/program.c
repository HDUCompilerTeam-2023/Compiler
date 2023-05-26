#include <hir2mir.h>
#include <symbol/func.h>
#include <program/gen.h>
#include <program/def.h>

void hir2mir_program_gen(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        hir2mir_func_gen(p_func, p_program);
    }
    program_mir_set_vmem_id(p_program);
    program_hir_drop(p_program);
}
