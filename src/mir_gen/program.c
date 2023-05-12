#include <mir/program.h>
#include <mir_gen.h>
#include <symbol_gen.h>

p_mir_program mir_program_gen(size_t func_cnt) {
    p_mir_program p_program = malloc(sizeof(*p_program));
    *p_program = (mir_program) {
        .func_cnt = func_cnt,
        .func_table = mir_func_table_gen(func_cnt),
        .vmem_list = list_head_init(&p_program->vmem_list),
        .p_store = NULL,
    };
    return p_program;
}

void mir_program_vmem_add(p_mir_program p_program, p_mir_vmem p_vmem) {
    list_add_prev(&p_vmem->node, &p_program->vmem_list);
}
void mir_program_set_vmem_id(p_mir_program p_program) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_program->vmem_list) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        p_vmem->id = id++;
    }
}

void mir_program_drop(p_mir_program p_program) {
    assert(p_program);
    mir_func_table_drop(p_program->func_table, p_program->func_cnt);
    while (!list_head_alone(&p_program->vmem_list)) {
        p_mir_vmem p_vmem = list_entry(p_program->vmem_list.p_next, mir_vmem, node);
        list_del(&p_vmem->node);
        free(p_vmem);
    }
    symbol_store_drop(p_program->p_store);
    free(p_program);
}
