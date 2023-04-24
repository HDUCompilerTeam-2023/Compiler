#include <mir/program.h>
#include <mir_gen.h>
#include <symbol_gen.h>

p_mir_program mir_program_gen(size_t func_cnt)
{
    p_mir_program p_program = malloc(sizeof(*p_program));
    *p_program = (mir_program){
        .func_cnt = func_cnt,
        .func_table = mir_func_table_gen(func_cnt),
        .p_store = NULL,
    };
    return p_program;
}


void mir_program_drop(p_mir_program p_program)
{
    assert(p_program);
    mir_func_table_drop(p_program->func_table, p_program->func_cnt);
    symbol_store_drop(p_program->p_store);
    free(p_program);
}