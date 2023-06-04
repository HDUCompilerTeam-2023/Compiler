#include <mir/vmem.h>
#include <mir_print.h>

#include <stdio.h>
#include <symbol/type.h>
#include <symbol/var.h>

void mir_vmem_print(p_mir_vmem p_vmem) {
    assert(p_vmem);
    if (!list_head_alone(&p_vmem->p_type->array)) {
        printf("[%ld X ", p_vmem->p_type->size);
        mir_basic_type_print(p_vmem->p_type->basic);
        printf("]");
    }
    else {
        mir_basic_type_print(p_vmem->p_type->basic);
    }
    for (size_t i = 0; i < p_vmem->p_type->ref_level; ++i) {
        printf("*");
    }

    if (p_vmem->p_var && p_vmem->p_var->is_global) {
        printf(" @");
    }
    else {
        printf(" $");
    }
    printf("%ld", p_vmem->id);
    if (p_vmem->p_var) {
        printf("_%s", p_vmem->p_var->name);
    }
}
