#include <mir_print.h>
#include <mir/symbol.h>

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
        mir_basic_type_print(p_type->basic);
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

void mir_symbol_print(p_mir_symbol p_mir_sym)
{
    assert(p_mir_sym);
    mir_symbol_type_print(p_mir_sym->p_type);

    if (p_mir_sym->kind == global_var){
        printf("@%s ", p_mir_sym->name);
    }
    else if(p_mir_sym->kind == temp_var)
        printf("%%%ld(temp) ", p_mir_sym->id);
    else{
        switch (p_mir_sym->p_type->kind) {
            case type_param:
                printf("%ld(param) ", p_mir_sym->id);
                break;
            case type_var:
                printf(" %ld(local) ", p_mir_sym->id);
                break;
            default:
                assert(0);
        }
    }
}