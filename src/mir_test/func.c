#include <mir.h>
#include <mir/basic_block.h>
#include <mir/func.h>
#include <mir/instr.h>
#include <mir/operand.h>
#include <mir/param.h>
#include <mir/program.h>
#include <mir/temp_sym.h>
#include <mir_port.h>

#include <mir_test.h>

#include <util.h>

#include <symbol/store.h>
#include <symbol/sym.h>
#include <symbol/type.h>

#include <stdbool.h>
#include <stdio.h>

memory_type mir_func_test(p_mir_func p_func, memory_stack **global_stack, memory_stack **top_stack) {
    assert(p_func);
    p_list_head p_node;
    p_symbol_type p_param_type = p_func->p_func_sym->p_type->p_params;
    list_for_each(p_node, &p_func->p_func_sym->variable) {
        if (!p_param_type) break;
        p_param_type = p_param_type->p_params;
    }
    while (p_node != &p_func->p_func_sym->variable) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        (*top_stack)->pos[p_sym->id] = (*top_stack)->mem_cnt;
        if (p_sym->p_init) {
            for (size_t i = 0; i < p_sym->p_init->size; ++i)
                if (p_sym->p_init->basic == type_int) {
                    (*top_stack)->mem[(*top_stack)->mem_cnt].i = p_sym->p_init->memory[i].i;
                    (*top_stack)->mem[(*top_stack)->mem_cnt++].type = I32;
                }
                else {
                    (*top_stack)->mem[(*top_stack)->mem_cnt].f = p_sym->p_init->memory[i].f;
                    (*top_stack)->mem[(*top_stack)->mem_cnt++].type = F32;
                }
        }
        else {
            p_symbol_type p_type = p_sym->p_type;
            while (p_type->kind != type_var)
                p_type = p_type->p_item;
            basic_type b_type = p_type->basic;
            for (size_t i = 0; i < p_sym->p_type->size; ++i)
                if (b_type == type_int) {
                    (*top_stack)->mem[(*top_stack)->mem_cnt].i = 0;
                    (*top_stack)->mem[(*top_stack)->mem_cnt++].type = I32;
                }
                else {
                    (*top_stack)->mem[(*top_stack)->mem_cnt].f = 0;
                    (*top_stack)->mem[(*top_stack)->mem_cnt++].type = F32;
                }
        }
        p_node = p_node->p_next;
    }
    p_list_head p_list = mir_func_get_basic_block_entry(p_func);
    memory_type re = mir_basic_block_test(p_list, &(*global_stack), &(*top_stack));
    stack_pop(&(*top_stack));
    return re;
}