#include <mir/operand.h>
#include <mir/temp_sym.h>

#include <mir.h>

#include <mir_test.h>

#include <util.h>

#include <symbol.h>
#include <symbol/sym.h>
#include <symbol/type.h>

memory_type mir_operand_data_get(p_mir_operand p_operand, const memory_stack *global_stack, const memory_stack *stack) {
    memory_type re = {};
    switch (p_operand->kind) {
    case immedicate_int_val:
        re.i = p_operand->intconst;
        re.type = I32;
        break;
    case immedicate_float_val:
        re.f = p_operand->floatconst;
        re.type = F32;
        break;
    case immedicate_void_val:
        break;
    case temp_var:
        return stack->mem[stack->mem_cnt + p_operand->p_temp_sym->id];

    case declared_var:
        if (p_operand->p_sym->is_global) {
            if (p_operand->p_sym->p_type->kind == type_var)
                return global_stack->mem[global_stack->pos[p_operand->p_sym->id]];
            else {
                re.i = global_stack->pos[p_operand->p_sym->id];
                re.type = I32;
            }
        }
        else {
            if (p_operand->p_sym->p_type->kind == type_var)
                return stack->mem[stack->pos[p_operand->p_sym->id]];
            else {
                re.i = stack->pos[p_operand->p_sym->id];
                re.type = I32;
            }
        }
        break;
    }
    return re;
}
