#include <mir_print.h>
#include <mir/program.h>
#include <mir/instr.h>
#include <mir/func.h>

#include <stdio.h>

void mir_program_print(p_mir_program p_program)
{
    assert(p_program);
    printf("=== mir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->globalvar_init_list){
        p_mir_instr p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }

    list_for_each(p_node, &p_program->func){
        p_mir_func p_func = list_entry(p_node, mir_func, node);
        mir_func_print(p_func);
    }
    printf(" === mir program end ===\n");
}