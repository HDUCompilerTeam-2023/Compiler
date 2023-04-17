
#include <mir_print.h>
#include <mir/operand.h>

#include <stdio.h>

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
        printf("]");
        p_type = p_mir_type;
        while (p_type->kind == type_arrary){ 
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
            printf(" ");
            if (p_operand->b_type == type_int) 
                printf("%d ", p_operand->intconst);
            else if (p_operand->b_type == type_float)
                printf("%f ", p_operand->floatconst);
            // else ?
            break;
        case global_var:
        case func_var:
            mir_symbol_type_print(p_operand->p_type);
            printf("@%s ", p_operand->name);
            break;
        case local_var:
            mir_symbol_type_print(p_operand->p_type);
            printf("%%l%ld ", p_operand->id);
            break;
        case temp_var_array:
            mir_symbol_type_print(p_operand->p_type);
            printf("%%t%ld ", p_operand->id);
            break;
        case temp_var_basic:
            mir_basic_type_print(p_operand->b_type);
            printf(" ");
            printf("%%t%ld ", p_operand->id);
            break;
    }
}

void mir_operand_list_print(p_mir_operand_list p_list)
{
    p_list_head p_node;
    list_for_each(p_node, &p_list->operand){
        p_mir_operand p_operand = list_entry(p_node, mir_operand, node);
        mir_operand_print(p_operand);
        printf("\n");
    }
}


