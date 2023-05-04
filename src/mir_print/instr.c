#include <mir_print.h>
#include <mir/instr.h>
#include <mir/basic_block.h>
#include <mir/func.h>
#include <stdio.h>

#include <symbol/sym.h>
void mir_instr_print(p_mir_instr p_instr)
{
    assert(p_instr);
    printf("    ");
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
            mir_binary_instr_print(p_instr->irkind, &p_instr->mir_binary);
            break;
        case mir_minus_op:
        case mir_not_op:
        case mir_int2float_op:
        case mir_float2int_op:
        case mir_val_assign:
            mir_unary_instr_print(p_instr->irkind, &p_instr->mir_unary);
            break;
        case mir_ret:
            mir_ret_instr_print(&p_instr->mir_ret);
            break;
        case mir_br:
            mir_br_instr_print(&p_instr->mir_br);
            break;
        case mir_condbr:
            mir_condbr_instr_print(&p_instr->mir_condbr);
            break;
        case mir_call:
            mir_call_instr_print(&p_instr->mir_call);
            break;
        case mir_array:
            mir_array_instr_print(&p_instr->mir_array);
            break;
        case mir_array_assign:
            mir_array_assign_instr_print(&p_instr->mir_array_assign);
    }
    printf("\n");
}

void mir_binary_instr_print(mir_instr_type instr_type, p_mir_binary_instr p_instr)
{
    mir_operand_print(p_instr->p_des);
    printf("= ");
    mir_operand_print(p_instr->p_src1);
    switch (instr_type) {
        case mir_add_op:
            printf("+ ");
            break;
        case mir_sub_op:
            printf("- ");
            break;
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
    mir_operand_print(p_instr->p_src2);
}

void mir_unary_instr_print(mir_instr_type instr_type, p_mir_unary_instr p_instr)
{
    mir_operand_print(p_instr->p_des);
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
        case mir_val_assign:
            break;
        default:
            assert(0);
    }
    mir_operand_print(p_instr->p_src);
}

void mir_ret_instr_print(p_mir_ret_instr p_instr)
{
    printf("ret ");
    mir_operand_print(p_instr->p_ret);
}

void mir_br_instr_print(p_mir_br_instr p_instr)
{
    printf("br ");
    printf("b%ld", p_instr->p_target->block_id);
}

void mir_condbr_instr_print(p_mir_condbr_instr p_instr)
{
    printf("br ");
    mir_operand_print(p_instr->p_cond);
    printf(", ");
    printf("b%ld, ", p_instr->p_target_true->block_id);
    printf("b%ld", p_instr->p_target_false->block_id);
}

void mir_call_instr_print(p_mir_call_instr p_instr)
{
    mir_operand_print(p_instr->p_des);
    printf("= ");
    printf("@%s", p_instr->p_func->p_func_sym->name);
    mir_param_list_print(p_instr->p_param_list);
}

void mir_array_instr_print(p_mir_array_instr p_instr)
{
    mir_operand_print(p_instr->p_des);
    printf("= ");
    mir_operand_print(p_instr->p_array);
    printf("[");
    mir_operand_print(p_instr->p_offset);
    printf("]");
}

void mir_array_assign_instr_print(p_mir_array_assign_instr p_instr)
{
    mir_operand_print(p_instr->p_array);
    printf("[ ");
    mir_operand_print(p_instr->p_offset);
    printf("] ");
    printf("= ");
    mir_operand_print(p_instr->p_src);
}