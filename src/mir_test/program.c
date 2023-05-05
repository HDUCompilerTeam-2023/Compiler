#include <mir/func.h>
#include <mir/program.h>

#include <mir_test.h>

#include <util.h>

#include <symbol/store.h>
#include <symbol/sym.h>
#include <symbol/type.h>

#include <stdio.h>

memory_stack *global_stack, *top_stack;

void mir_program_test(const p_mir_program p_program) {
    assert(p_program);
    printf("\n === mir test start ===\n");
    stack_push(&top_stack);
    global_stack = top_stack;
    p_list_head p_node;
    list_for_each(p_node, &p_program->p_store->variable) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        global_stack->pos[p_sym->id] = global_stack->mem_cnt;
        if (p_sym->is_def) {
            for (size_t i = 0; i < p_sym->p_init->size; ++i) {
                if (p_sym->p_init->basic == type_int) {
                    global_stack->mem[global_stack->mem_cnt].i = p_sym->p_init->memory[i].i;
                    global_stack->mem[global_stack->mem_cnt++].type = I32;
                }
                else {
                    global_stack->mem[global_stack->mem_cnt].f = p_sym->p_init->memory[i].f;
                    global_stack->mem[global_stack->mem_cnt++].type = F32;
                }
            }
        }
        else {
            global_stack->mem_cnt += p_sym->p_type->size;
            p_symbol_type p_type = p_sym->p_type;
            while (p_type->kind != type_var)
                p_type = p_type->p_item;
            basic_type b_type = p_type->basic;
            top_stack->pos[p_sym->id] = top_stack->mem_cnt;
            for (size_t i = 0; i < p_sym->p_type->size; ++i)
                if (b_type == type_int) {
                    global_stack->mem[global_stack->mem_cnt].i = 0;
                    global_stack->mem[global_stack->mem_cnt++].type = I32;
                }
                else {
                    global_stack->mem[global_stack->mem_cnt].f = 0;
                    global_stack->mem[global_stack->mem_cnt++].type = F32;
                }
        }
    }
    for (size_t i = 0; i < p_program->func_cnt; i++)
        if (!strcmp((p_program->func_table + i)->p_func_sym->name, "main")) {
            stack_push(&top_stack);
            mir_func_test(p_program->func_table + i, &global_stack, &top_stack);
        }
    stack_pop(&top_stack);
    printf(" === mir test end ===\n");
}