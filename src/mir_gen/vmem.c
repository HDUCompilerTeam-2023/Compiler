#include <mir/vmem.h>
#include <mir_gen.h>

#include <symbol/sym.h>
#include <symbol/type.h>

p_mir_vmem mir_vmem_temp_gen(basic_type b_type, size_t ref_level) {
    if (b_type == type_void)
        return NULL;

    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_sym = NULL,
        .b_type = b_type,
        .ref_level = ref_level,
        .is_array = false,
        .size = 1,
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}

p_mir_vmem mir_vmem_sym_gen(p_symbol_sym p_sym) {
    assert(p_sym->p_type->kind == type_arrary || p_sym->p_type->kind == type_var);

    p_symbol_type p_r_type = p_sym->p_type;
    size_t ref_level = 0;
    while (p_r_type->kind == type_arrary && p_r_type->size == 0) {
        p_r_type = p_r_type->p_item;
        ++ref_level;
    }

    while (p_r_type->kind == type_arrary)
        p_r_type = p_r_type->p_item;

    bool is_array = p_sym->p_type->kind == type_arrary && p_sym->p_type->size != 0;

    p_mir_vmem p_vmem = malloc(sizeof(*p_vmem));
    *p_vmem = (mir_vmem) {
        .p_sym = p_sym,
        .b_type = p_r_type->basic,
        .ref_level = ref_level,
        .is_array = is_array,
        .size = is_array ? p_sym->p_type->size : 1,
        .id = 0,
        .node = list_head_init(&p_vmem->node),
    };

    return p_vmem;
}
void mir_vmem_drop(p_mir_vmem p_vmem) {
    free(p_vmem);
}
