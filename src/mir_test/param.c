#include <mir.h>
#include <mir/operand.h>
#include <mir/param.h>
#include <mir/temp_sym.h>

#include <mir_test.h>

#include <symbol/sym.h>
#include <symbol/type.h>

#include <util.h>

void mir_param_list_test(p_mir_param_list p_param_list, memory_stack *global_stack, memory_stack *top_stack) {
    assert(p_param_list);
    int id = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param) {
        p_mir_param p_param = list_entry(p_node, mir_param, node);
        top_stack->pos[id++] = top_stack->mem_cnt;
        p_mir_operand p_operand = p_param->p_param;
        switch (p_operand->kind) {
        case immedicate_int_val:
            top_stack->mem[top_stack->mem_cnt].i = p_operand->intconst;
            top_stack->mem[top_stack->mem_cnt++].type = I32;
            break;
        case immedicate_float_val:
            top_stack->mem[top_stack->mem_cnt].f = p_operand->floatconst;
            top_stack->mem[top_stack->mem_cnt++].type = F32;
            break;
        case immedicate_void_val:
            break;
        case temp_var:
            if (p_operand->p_temp_sym->is_pointer) {
                memory_type p_arry = mir_operand_data_get(p_operand, global_stack, top_stack->prev);
                if (p_arry.type < 0)
                    for (int i = 0; i > p_arry.type; --i)
                        top_stack->mem[top_stack->mem_cnt++] = global_stack->mem[p_arry.i - i];
                else
                    for (int i = 0; i < p_arry.type; ++i)
                        top_stack->mem[top_stack->mem_cnt++] = top_stack->prev->mem[p_arry.i + i];
            }
            else
                top_stack->mem[top_stack->mem_cnt++] = mir_operand_data_get(p_operand, global_stack, top_stack->prev);
            break;
        case declared_var:
            if (p_operand->p_sym->is_global)
                for (int i = 0; i < p_operand->p_sym->p_type->size; i++)
                    top_stack->mem[top_stack->mem_cnt++] = global_stack->mem[global_stack->pos[p_operand->p_sym->id] + i];
            else
                for (int i = 0; i < p_operand->p_sym->p_type->size; i++)
                    top_stack->mem[top_stack->mem_cnt++] = top_stack->prev->mem[top_stack->prev->pos[p_operand->p_sym->id] + i];
        }
    }
}