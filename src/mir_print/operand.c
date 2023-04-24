
#include <mir_print.h>
#include <mir/operand.h>

#include <stdio.h>
#include <symbol/type.h>
#include <symbol/sym.h>
#include <mir/temp_sym.h>
void mir_basic_type_print(basic_type b_type)
{
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

void mir_symbol_type_print(p_symbol_type p_mir_type){
    assert(p_mir_type);
    if(p_mir_type->kind == type_arrary){
        p_symbol_type p_type = p_mir_type;
        while(p_type->kind == type_arrary)p_type = p_type->p_item;
        printf("[%ld X ", p_mir_type->size);
        mir_basic_type_print(p_type->basic);
        printf("]*");
    }
    else if (p_mir_type->kind == type_param)
        mir_symbol_type_print(p_mir_type->p_item);
    else
        mir_basic_type_print(p_mir_type->basic);
    printf(" ");
}

void mir_operand_print(p_mir_operand p_operand)
{
    switch (p_operand->kind) {
        case immedicate_int_val:
            printf("i32 ");
            printf("%d ", p_operand->intconst);
            break;
        case immedicate_float_val:
            printf("f32 ");
            printf("%f ", p_operand->floatconst);
            break;
        case immedicate_void_val:
            printf("void");
            break;
        case temp_var:
            mir_temp_sym_print(p_operand->p_temp_sym);
            break;
        case declared_var:
            mir_symbol_type_print(p_operand->p_sym->p_type);
            if (p_operand->p_sym->is_global) {
                printf("@%s ", p_operand->p_sym->name);
            }
            else {
                printf("%%l%ld ", p_operand->p_sym->id);
            }
            break;

    }
}



