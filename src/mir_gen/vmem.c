#include <symbol_gen/type.h>
#include <mir/vmem.h>
#include <mir_gen.h>

#include <symbol/var.h>
#include <symbol/type.h>

p_mir_vmem mir_vmem_temp_gen(basic_type b_type, size_t ref_level) {
    if (b_type == type_void)
        return NULL;

    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_var = NULL,
        .b_type = b_type,
        .ref_level = ref_level,
        .is_array = false,
        .size = 1,
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}

p_mir_vmem mir_vmem_sym_gen(p_symbol_var p_var) {
    bool is_array = !list_head_alone(&p_var->p_type->array) && p_var->p_type->ref_level == 0;

    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_var = p_var,
        .b_type = p_var->p_type->basic,
        .ref_level = p_var->p_type->ref_level,
        .is_array = is_array,
        .size = is_array ? symbol_type_get_size(p_var->p_type) : 1,
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}
void mir_vmem_drop(p_mir_vmem p_vmem) {
    free(p_vmem);
}
