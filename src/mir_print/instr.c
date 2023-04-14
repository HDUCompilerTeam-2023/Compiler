
#include <mir_print.h>
#include <mir/instr.h>
#include <stdio.h>

void mir_instr_print(p_mir_instr p_instr)
{
    assert(p_instr);
    printf("i%ld:  ", p_instr->index);
    switch (p_instr->irkind) {
        case mir_add_op:
        case mir_sub_op:
        case mir_mul_op:
        case mir_div_op:
        case mir_mod_op:
        case mir_and_op:
        case mir_or_op:
        case mir_eq_op:
        case mir_neq_op:
        case mir_l_op:
        case mir_leq_op:
        case mir_g_op:
        case mir_geq_op:
            mir_binary_instr_print(p_instr->irkind, p_instr);
            break;
        case mir_minus_op:
        case mir_not_op:
        case mir_int2float_op:
        case mir_float2int_op:
        case mir_assign:
            mir_unary_instr_print(p_instr->irkind, p_instr);
            break;
        case mir_ret:
            mir_ret_instr_print(p_instr);
            break;
        case mir_br:
            mir_br_instr_print(p_instr);
            break;
        case mir_condbr:
            mir_condbr_instr_print(p_instr);
            break;
        case mir_call:
            mir_call_instr_print(p_instr);
            break;
        case mir_array:
            mir_array_instr_print(p_instr);
            break;
    }
    printf("\n");
}

void mir_binary_instr_print(mir_instr_type instr_type, p_mir_instr p_instr)
{
    mir_symbol_print(p_instr->mir_binary.p_des);
    printf("= ");
    mir_operand_print(p_instr->mir_binary.p_src1);
    switch (instr_type) {
        case mir_add_op:
            printf("+ ");
            break;
        case mir_sub_op:
            printf("- ");
        case mir_mul_op:
            printf("* ");
            break;
        case mir_div_op:
            printf("/ ");
            break;
        case mir_mod_op:
            printf("%% ");
            break;
        case mir_and_op:
            printf("&& ");
            break;
        case mir_or_op:
            printf("|| ");
            break;
        case mir_eq_op:
            printf("== ");
            break;
        case mir_neq_op:
            printf("!= ");
            break;
        case mir_l_op:
            printf("< ");
            break;
        case mir_leq_op:
            printf("<= ");
            break;
        case mir_g_op:
            printf("> ");
            break;
        case mir_geq_op:
            printf(">= ");
            break;
        default:
            assert(0);
    }
    mir_operand_print(p_instr->mir_binary.p_src2);
}

void mir_unary_instr_print(mir_instr_type instr_type, p_mir_instr p_instr)
{
    mir_symbol_print(p_instr->mir_unary.p_des);
    printf("= ");
    switch (instr_type) {
        case mir_minus_op:
            printf("- ");
            break;
        case mir_not_op:
            printf("! ");
            break;
        case mir_int2float_op:
            printf("(float) ");
            break;
        case mir_float2int_op:
            printf("(int) ");
            break;
        case mir_assign:
            break;
        default:
            assert(0);
    }
    mir_operand_print(p_instr->mir_unary.p_src);
}

void mir_ret_instr_print(p_mir_instr p_instr)
{
    printf("return ");
    mir_operand_print(p_instr->mir_ret.p_ret);
}

void mir_br_instr_print(p_mir_instr p_instr)
{
    printf("br ");
    printf("i%ld", p_instr->index);
}

void mir_condbr_instr_print(p_mir_instr p_instr)
{
    printf("br ");
    mir_operand_print(p_instr->mir_condbr.p_cond);
    printf(", ");
    printf("i%ld, ", p_instr->mir_condbr.p_target_true->index);
    printf("i%ld", p_instr->mir_condbr.p_target_false->index);
}

void mir_call_instr_print(p_mir_instr p_instr)
{
    mir_symbol_print(p_instr->mir_call.p_des);
    printf("= ");
    mir_param_list_print(p_instr->mir_call.p_param_list);
}

void mir_array_instr_print(p_mir_instr p_instr)
{
    mir_symbol_print(p_instr->mir_array.p_des);
    printf("= ");
    mir_symbol_print(p_instr->mir_array.p_array);
    printf("[");
    mir_operand_print(p_instr->mir_array.p_offset);
    printf("]");
}