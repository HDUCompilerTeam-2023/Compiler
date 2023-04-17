#include <mir_print.h>
#include <mir/func.h>
#include <stdio.h>
void mir_func_print(p_mir_func p_func)
{
    assert(p_func);
    printf("define ");
    mir_symbol_type_print(p_func->p_func_sym->p_type);
    printf("%s (", p_func->p_func_sym->name);
    p_list_head p_node;
    p_symbol_type p_param_type = p_func->p_func_sym->p_type->p_params;
    list_for_each(p_node, &p_func->p_func_sym->local) {
        if (!p_param_type) break;
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        mir_symbol_type_print(p_sym->p_type);
        printf("%%%ld", p_sym->id);
        p_param_type = p_param_type->p_params;
        if(p_param_type)printf(", ");
    }
    printf("){\n");
    mir_basic_block_print(p_func->p_basic_block);
    printf("}\n");
}