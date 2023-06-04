
#include <mir/operand.h>
#include <mir_print.h>

#include <mir/vreg.h>
#include <stdio.h>
#include <symbol/var.h>
#include <symbol/type.h>
#include <symbol_print.h>
void mir_basic_type_print(basic_type b_type) {
    switch (b_type) {
    case type_int:
        printf("i32");
        break;
    case type_float:
        printf("f32");
        break;
    case type_void:
        printf("void");
        break;
    case type_str:
        printf("str");
        break;
    }
}

void mir_symbol_type_print(p_symbol_type p_mir_type) {
    assert(p_mir_type);
    if (!list_head_alone(&p_mir_type->array) || p_mir_type->ref_level > 0) {
        printf("[%ld X ", p_mir_type->ref_level > 0 ? 0 : symbol_type_get_size(p_mir_type));
        mir_basic_type_print(p_mir_type->basic);
        printf("]*");
    }
    else
        mir_basic_type_print(p_mir_type->basic);
    printf(" ");
}

void mir_operand_print(p_mir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        mir_basic_type_print(p_operand->b_type);
        if (p_operand->ref_level > 0) {
            for (size_t i = 0; i < p_operand->ref_level; ++i) {
                printf("*");
            }
            printf(" (addr ");
            mir_vmem_print(p_operand->p_global_vmem);
            printf(") ");
        }
        else if (p_operand->b_type == type_int)
            printf(" %ld ", p_operand->intconst);
        else if (p_operand->b_type == type_float)
            printf(" %f ", p_operand->floatconst);
        else if (p_operand->b_type == type_str)
            symbol_str_print(p_operand->strconst);
        break;
    case reg:
        mir_vreg_print(p_operand->p_vreg);
        printf(" ");
        break;
    }
}
