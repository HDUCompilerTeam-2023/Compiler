#include "mir_gen/basic_block.h" // 这边有问题， 各模块之间应该如何包含？
#include <mir_print.h>
#include <mir/func.h>

#include <mir/instr.h>// 包含问题？
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
    p_param_type = p_func->p_func_sym->p_type->p_params;
    list_for_each(p_node, &p_func->p_func_sym->local) {
        if (!p_param_type) break;
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        printf("param ");
        mir_symbol_type_print(p_sym->p_type);
        printf("%%l%ld\n", p_sym->id);
        p_param_type = p_param_type->p_params;
    }

    while (p_node != &p_func->p_func_sym->local) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        printf("local ");
        mir_symbol_type_print(p_sym->p_type);
        printf("%%l%ld\n", p_sym->id);
        p_node = p_node->p_next;
    }

    mir_basic_block_visited_init(p_func->p_basic_block);
    mir_basic_block_print(p_func->p_basic_block);
    // 输出 return 块
    // 输出前驱节点
    printf("b%ld:", p_func->p_ret_block->block_id);
    if (!list_head_alone(&p_func->p_ret_block->p_prev_block_list->basic_block_list)) {
        printf("                        ; preds = ");
        p_list_head p_node;
        list_for_each(p_node, &p_func->p_ret_block->p_prev_block_list->basic_block_list){
            size_t id = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block->block_id;
            printf("b%ld", id);
            if(p_node->p_next != &p_func->p_ret_block->p_prev_block_list->basic_block_list)
                printf(", ");
        }
    }
    printf("\n");
    list_for_each(p_node, &p_func->p_ret_block->instr_list)
    {
        p_mir_instr p_instr = list_entry(p_node, mir_instr, node);
        mir_instr_print(p_instr);
    }
    printf("}\n");
}