#include <mir_print.h>
#include <mir/operand.h>

#include <stdio.h>

void mir_operand_print(p_mir_operand p_operand)
{
    switch (p_operand->kind) {
        case int_val:
            printf("i32 %d ", p_operand->intconst);
            break;
        case float_val:
            printf("f32 %f ", p_operand->floatconst);
            break;
        case void_val:
            printf("void ");
        case sym:
            mir_symbol_print(p_operand->p_sym);
    }
}


