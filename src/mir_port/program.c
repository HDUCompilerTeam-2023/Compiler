#include <mir_port/program.h>
#include <mir/program.h>
p_list_head mir_program_get_global_list(p_mir_program p_program)
{
    return p_program->p_globalvar_head;
}

p_list_head mir_program_get_func_head(p_mir_program p_program)
{
    return &p_program->func;
}