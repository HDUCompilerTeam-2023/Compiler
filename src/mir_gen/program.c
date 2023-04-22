#include <mir/program.h>
#include <mir_gen.h>
#include <symbol_gen.h>

p_mir_program mir_program_gen(void)
{
    p_mir_program p_program = malloc(sizeof(*p_program));
    *p_program = (mir_program){
        .func = list_head_init(&p_program->func),
        .p_globalvar_head = NULL,
        .p_store = NULL,
    };
    return p_program;
}

p_mir_program mir_program_func_add(p_mir_program p_program, p_mir_func p_func)
{
    list_add_prev(&p_func->node, &p_program->func);
    return p_program;
}

p_mir_program mir_program_global_set(p_mir_program p_program, p_list_head p_head)
{
    p_program->p_globalvar_head = p_head;
    return p_program;
}

void mir_program_drop(p_mir_program p_program)
{
    assert(p_program);
    while (!list_head_alone(&p_program->func)) {
        p_mir_func p_func = list_entry(p_program->func.p_next, mir_func, node);
        list_del(&p_func->node);
        mir_func_drop(p_func);
    }
    symbol_store_drop(p_program->p_store);
    free(p_program);
}