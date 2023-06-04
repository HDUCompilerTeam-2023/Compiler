#include <symbol_gen/type.h>
#include <mir/vmem.h>
#include <mir_gen.h>

#include <symbol/var.h>
#include <symbol/type.h>

p_mir_vmem mir_vmem_temp_gen(p_symbol_type p_type) {
    assert(p_type->basic != type_void);

    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_var = NULL,
        .p_type = p_type,
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}

p_mir_vmem mir_vmem_sym_gen(p_symbol_var p_var) {
    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_var = p_var,
        .p_type = symbol_type_copy(p_var->p_type),
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}
void mir_vmem_drop(p_mir_vmem p_vmem) {
    symbol_type_drop(p_vmem->p_type);
    free(p_vmem);
}
