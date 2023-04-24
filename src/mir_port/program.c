#include <mir_port/program.h>
#include <mir/program.h>

#include <symbol/store.h>
p_list_head mir_program_get_global_list(p_mir_program p_program)
{
    return &p_program->p_store->variable;
}

p_mir_func mir_program_get_func_table(p_mir_program p_program)
{
    return p_program->func_table;
}