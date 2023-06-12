#include <ir_print.h>
#include <stdio.h>
#include <symbol_print.h>

#include <program/def.h>
#include <program/use.h>
#include <symbol/func.h>
#include <symbol/str.h>
#include <symbol/var.h>

#include <ir/basic_block.h>

void program_variable_print(p_program p_program) {
    assert(p_program);
    printf("=== program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        printf("global ");
        symbol_init_print(p_var);
    }
    list_for_each(p_node, &p_program->constant) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        printf("constant ");
        symbol_init_print(p_var);
    }
    list_for_each(p_node, &p_program->string) {
        p_symbol_str p_str = list_entry(p_node, symbol_str, node);
        printf("string ");
        symbol_str_print(p_str);
    }
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        symbol_func_init_print(p_func);
    }
    printf("=== program end ===\n");
}

void program_ir_print(p_program p_program) {
    assert(p_program);
    printf("=== ir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        ;
        symbol_func_init_print(p_func);
        printf("{\n");
        p_list_head p_node;
        list_for_each(p_node, &p_func->param) {
            p_symbol_var p_var = list_entry(p_node, symbol_var, node);
            printf("param ");
            symbol_name_print(p_var);
            printf(" %%%ld", p_var->id);
            printf("\n");
        }
        list_for_each(p_node, &p_func->variable) {
            p_symbol_var p_var = list_entry(p_node, symbol_var, node);
            printf("local ");
            symbol_name_print(p_var);
            printf("\n");
        }
        list_for_each(p_node, &p_func->block) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
            ir_basic_block_print(p_basic_block);
        }
        printf("}\n");
    }
    printf("=== ir program end ===\n");
}
void program_ir_dom_info_print(p_program p_program) {
    printf("+++ dom_tree start +++\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        ;
        symbol_func_init_print(p_func);
        p_ir_basic_block p_basic_block = list_entry(p_func->block.p_next, ir_basic_block, node);
        ir_basic_block_dom_info_print(p_basic_block, 0);
    }
    printf("+++ dom_tree end +++\n");
}
