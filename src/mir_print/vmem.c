#include <mir/vmem.h>
#include <mir_print.h>

#include <stdio.h>
#include <symbol_print.h>
#include <symbol/var.h>

void mir_vmem_print(p_mir_vmem p_vmem) {
    assert(p_vmem);
    symbol_type_print(p_vmem->p_type);

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
