#include <mir/basic_block.h>
#include <mir/func.h>
#include <mir/instr.h>
#include <mir_print.h>
#include <stdio.h>

#include <symbol/sym.h>
void mir_instr_print(p_mir_instr p_instr) {
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
    case mir_val_assign:
        mir_unary_instr_print(p_instr->irkind, &p_instr->mir_unary);
        break;
    case mir_call:
        mir_call_instr_print(&p_instr->mir_call);
        break;
    case mir_alloca:
        mir_alloca_instr_print(&p_instr->mir_alloca);
        break;
    case mir_load:
        mir_load_instr_print(&p_instr->mir_load);
        break;
    case mir_store:
        mir_store_instr_print(&p_instr->mir_store);
    }
    printf("\n");
}

void mir_binary_instr_print(mir_instr_type instr_type, p_mir_binary_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    mir_operand_print(p_instr->p_src1);
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
    mir_operand_print(p_instr->p_src2);
}

void mir_unary_instr_print(mir_instr_type instr_type, p_mir_unary_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    switch (instr_type) {
    case mir_minus_op:
        printf("- ");
        break;
    case mir_not_op:
        printf("! ");
        break;
    case mir_val_assign:
        break;
    default:
        assert(0);
    }
    mir_operand_print(p_instr->p_src);
}

void mir_call_instr_print(p_mir_call_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = ");
    printf("@%s", p_instr->p_func->name);
    mir_param_list_print(p_instr->p_param_list);
}

void mir_load_instr_print(p_mir_load_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = load ");
    mir_operand_print(p_instr->p_addr);
    if (p_instr->p_offset) {
        mir_operand_print(p_instr->p_offset);
    }
}

void mir_alloca_instr_print(p_mir_alloca_instr p_instr) {
    mir_vreg_print(p_instr->p_des);
    printf(" = alloca ");
    mir_vmem_print(p_instr->p_vmem);
}

void mir_store_instr_print(p_mir_store_instr p_instr) {
    printf("store ");
    mir_operand_print(p_instr->p_addr);
    if (p_instr->p_offset) {
        mir_operand_print(p_instr->p_offset);
    }
    printf("= ");
    mir_operand_print(p_instr->p_src);
}
