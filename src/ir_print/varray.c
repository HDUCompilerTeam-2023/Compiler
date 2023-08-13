#include <ir/varray.h>
#include <ir/vreg.h>
#include <ir_print.h>
#include <stdio.h>
#include <symbol/var.h>

void ir_varray_use_print(p_ir_varray_use p_use) {
    assert(p_use);
    ir_varray_print(p_use->p_varray_use);
}
void ir_varray_print(p_ir_varray p_varray) {
    assert(p_varray);
    if (p_varray->p_base->is_vmem) {
        if (p_varray->p_base->p_vmem_base->is_global)
            printf("@");
        else
            printf("$");
        printf("A%ld", p_varray->p_base->p_vmem_base->id);
        if (p_varray->p_base->p_vmem_base->name)
            printf("_%s", p_varray->p_base->p_vmem_base->name);
        printf(".%ld", p_varray->id);
    }
    else
        printf("%%A%ld.%ld", p_varray->p_base->p_param_base->id, p_varray->id);
}
void ir_varray_def_pair_print(p_ir_varray_def_pair p_pair) {
    assert(p_pair->p_des);
    assert(p_pair->p_src);
    ir_varray_print(p_pair->p_des);
    printf(" = update ");
    ir_varray_use_print(p_pair->p_src);
}