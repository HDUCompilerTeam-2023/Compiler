#include "mir_gen/basic_block.h" // 这边有问题， 各模块之间应该如何包含？
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
        printf("%%l%ld", p_sym->id);
        p_param_type = p_param_type->p_params;
        if(p_param_type)printf(", ");
    }
    printf("){\n");
    mir_basic_block_visited_init(p_func->p_basic_block);
    mir_basic_block_print(p_func->p_basic_block);
    printf("}\n");
}