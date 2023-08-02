#include <stdio.h>
#include <symbol_print.h>
#include <ir_print/vreg.h>

#include <symbol/func.h>
#include <symbol/type.h>
#include <ir/vreg.h>

void symbol_func_name_print(p_symbol_func p_func) {
    printf("%s", p_func->name);
}

void symbol_func_init_print(p_symbol_func p_func) {
    printf("define ");
    symbol_basic_type_print(p_func->ret_type);
    printf(" ");
    symbol_func_name_print(p_func);
    printf(" (");
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_param_reg = list_entry(p_node, ir_vreg, node);
        ir_vreg_print(p_param_reg);
        if (p_node != p_func->param_reg_list.p_prev) printf(", ");
    }
    printf(")\n");
}
