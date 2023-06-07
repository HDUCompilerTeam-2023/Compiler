#include <hir_print.h>
#include <stdio.h>

#include <hir/exp.h>
#include <symbol_print.h>

#include <symbol/func.h>
#include <symbol/str.h>
#include <symbol/type.h>
#include <symbol/var.h>

void hir_exp_print(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_binary:
        switch (p_exp->b_op) {
        case hir_exp_op_add:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("+ ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_sub:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("- ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_mul:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("* ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_div:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("/ ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_mod:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("%% ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_eq:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("== ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_neq:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("!= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_l:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("< ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_leq:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("<= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_g:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("> ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_geq:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf(">= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        }
        break;
    case hir_exp_unary:
        switch (p_exp->u_op) {
        case hir_exp_op_minus:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("minus ");
            hir_exp_print(p_exp->p_src);
            break;
        }
        break;
    case hir_exp_ulogic:
        switch (p_exp->u_op) {
        case hir_exp_op_bool_not:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("! ");
            hir_exp_print(p_exp->p_src);
            break;
        }
        break;
    case hir_exp_logic:
        switch (p_exp->l_op) {
        case hir_exp_op_bool_and:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("&& ");
            hir_exp_print(p_exp->p_bool_1);
            printf(" ");
            hir_exp_print(p_exp->p_bool_2);
            break;
        case hir_exp_op_bool_or:
            if (p_exp->p_type->basic == type_float) printf("f");
            printf("|| ");
            hir_exp_print(p_exp->p_bool_1);
            printf(" ");
            hir_exp_print(p_exp->p_bool_2);
            break;
        }
        break;
    case hir_exp_call:
        assert(p_exp->p_func);
        symbol_func_name_print(p_exp->p_func);
        printf("(");
        hir_param_list_print(p_exp->p_param_list);
        printf(")");
        break;
    case hir_exp_ptr:
        symbol_name_print(p_exp->p_var);
        break;
    case hir_exp_gep:
        if (p_exp->is_element) {
            hir_exp_print(p_exp->p_addr);
            printf("[");
            hir_exp_print(p_exp->p_offset);
            printf("]");
        }
        else {
            printf("(");
            hir_exp_print(p_exp->p_addr);
            printf(" + ");
            hir_exp_print(p_exp->p_offset);
            printf(")");
        }
        break;
    case hir_exp_load:
        printf("*(");
        hir_exp_print(p_exp->p_ptr);
        printf(")");
        break;
    case hir_exp_num:
        if (p_exp->p_type->basic == type_float)
            printf("%lf", p_exp->floatconst);
        else if (p_exp->p_type->basic == type_str)
            symbol_str_print(p_exp->p_str);
        else
            printf("%ld", p_exp->intconst);
        break;
    case hir_exp_use:
        hir_exp_print(p_exp->p_exp);
        break;
    }
}
