#include <hir_print.h>
#include <stdio.h>

#include <hir/exp.h>
#include <symbol_print.h>

#include <symbol/type.h>
#include <symbol/sym.h>
#include <symbol/str.h>

void hir_exp_print(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_exec:
        switch (p_exp->op) {
        case hir_exp_op_assign:
            hir_exp_print(p_exp->p_src_1);
            printf(" = ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_add:
            if (p_exp->basic == type_float) printf("f");
            printf("+ ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_sub:
            if (p_exp->basic == type_float) printf("f");
            printf("- ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_mul:
            if (p_exp->basic == type_float) printf("f");
            printf("* ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_div:
            if (p_exp->basic == type_float) printf("f");
            printf("/ ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_mod:
            if (p_exp->basic == type_float) printf("f");
            printf("%% ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_and:
            if (p_exp->basic == type_float) printf("f");
            printf("&& ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_or:
            if (p_exp->basic == type_float) printf("f");
            printf("|| ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_eq:
            if (p_exp->basic == type_float) printf("f");
            printf("== ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_neq:
            if (p_exp->basic == type_float) printf("f");
            printf("!= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_l:
            if (p_exp->basic == type_float) printf("f");
            printf("< ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_leq:
            if (p_exp->basic == type_float) printf("f");
            printf("<= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_g:
            if (p_exp->basic == type_float) printf("f");
            printf("> ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_geq:
            if (p_exp->basic == type_float) printf("f");
            printf(">= ");
            hir_exp_print(p_exp->p_src_1);
            printf(" ");
            hir_exp_print(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_not:
            if (p_exp->basic == type_float) printf("f");
            printf("! ");
            hir_exp_print(p_exp->p_src_1);
            break;
        case hir_exp_op_minus:
            if (p_exp->basic == type_float) printf("f");
            printf("minus ");
            hir_exp_print(p_exp->p_src_1);
            break;
        }
        break;
    case hir_exp_call:
        hir_func_call_print(p_exp->p_func, p_exp->p_param_list);
        break;
    case hir_exp_val:
        if (p_exp->p_offset) {
            if (p_exp->p_type->kind == type_var) {
                symbol_name_print(p_exp->p_sym);
                printf("[");
                hir_exp_print(p_exp->p_offset);
                printf("]");
            }
            else {
                printf("+ ");
                symbol_name_print(p_exp->p_sym);
                printf(" ");
                hir_exp_print(p_exp->p_offset);
            }
        }
        else {
            symbol_name_print(p_exp->p_sym);
        }
        break;
    case hir_exp_num:
        if (p_exp->basic == type_float) printf("%lf", p_exp->floatconst);
        else printf("%ld", p_exp->intconst);
        break;
    case hir_exp_str:
        symbol_str_print(p_exp->p_str);
        break;
    }
}