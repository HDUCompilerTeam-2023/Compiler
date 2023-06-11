
#include <mir/operand.h>
#include <mir_print.h>

#include <mir/vreg.h>
#include <stdio.h>
#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_print.h>

void mir_operand_print(p_mir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        symbol_type_print(p_operand->p_type);
        if (p_operand->p_type->ref_level > 0) {
            printf(" (addr ");
            symbol_name_print(p_operand->p_vmem);
            printf(")");
        }
        else if (p_operand->p_type->basic == type_int)
            printf(" %ld", p_operand->intconst);
        else if (p_operand->p_type->basic == type_float)
            printf(" %f", p_operand->floatconst);
        else if (p_operand->p_type->basic == type_str)
            symbol_str_print(p_operand->strconst);
        break;
    case reg:
        mir_vreg_print(p_operand->p_vreg);
        break;
    }
}
