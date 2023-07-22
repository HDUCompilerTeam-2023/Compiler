
#include <ir/operand.h>
#include <ir_print.h>

#include <ir/vreg.h>
#include <stdio.h>
#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_print.h>

void ir_operand_print(p_ir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        symbol_type_print(p_operand->p_type);
        if (p_operand->p_type->ref_level > 0) {
            printf(" (addr ");
            symbol_name_print(p_operand->p_vmem);
            if (p_operand->offset)
                printf(", %d", p_operand->offset);
            printf(")");
        }
        else if (p_operand->p_type->basic == type_i32)
            printf(" %d", p_operand->i32const);
        else if (p_operand->p_type->basic == type_f32)
            printf(" %f", p_operand->f32const);
        else if (p_operand->p_type->basic == type_str)
            symbol_str_print(p_operand->strconst);
        break;
    case reg:
        ir_vreg_print(p_operand->p_vreg);
        break;
    }
}
