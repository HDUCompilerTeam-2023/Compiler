
#include <mir_print.h>
#include <mir/operand.h>

#include <stdio.h>

void mir_basic_type_print(basic_type b_type)
{
    switch (b_type) {
        case type_int:
            printf("i32");
        case type_float:
            printf("f32");
        case type_void:
            printf("void");
        case type_str:
            printf("str");
    }
}

void mir_symbol_type_print(p_symbol_type p_mir_type){
    
    if(p_mir_type->kind == type_arrary){
        p_symbol_type p_type = p_mir_type;
        while(p_type == type_arrary)p_type = p_mir_type->p_item;
        printf("[%ld X ", p_mir_type->size);
        mir_basic_type_print(p_type->basic);
        printf("]");
        p_type = p_mir_type;
        while (p_type == type_arrary){ 
            printf("*");
            p_type = p_type->p_item;
        }
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
        case immedicate_val:
            mir_basic_type_print(p_operand->b_type);
            if (p_operand->b_type == type_int) 
                printf("%d ", p_operand->intconst);
            else if (p_operand->b_type == type_float)
                printf("%f ", p_operand->floatconst);
            // else ?
            break;
        case global_var:
            mir_symbol_type_print(p_operand->p_type);
            printf("@%s ", p_operand->name);
        case local_var:
            mir_symbol_type_print(p_operand->p_type);
            printf("%%%ld", p_operand->id);
        case temp_var:
            mir_symbol_type_print(p_operand->p_type);
            printf("%%%ld", p_operand->id);
    }
}


