
#include <mir_print.h>
#include <mir/program.h>
#include <mir/instr.h>
#include <mir/func.h>

#include <hir_print/exp.h> // how to not include it ?
#include <stdio.h>

#include <symbol/sym.h>
#include <symbol/store.h>
void mir_program_print(p_mir_program p_program)
{
    assert(p_program);
    printf("=== mir program start ===\n");
    p_list_head p_node;
    list_for_each(p_node, &p_program->p_store->variable){
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        printf("global ");
        mir_symbol_type_print(p_sym->p_type);
        printf("%s ", p_sym->name);
        if (p_sym->is_def){
            printf("= {");
            for(size_t i = 0; i < p_sym->p_init->size; i ++){
                if(i > 0) printf(", ");
                if (p_sym->p_init->basic == type_int)
                    printf("%ld", p_sym->p_init->memory[i].i);
                else
                    printf("%lf", p_sym->p_init->memory[i].f);
            }
            printf("}");
        }
        printf("\n");
    }

    for (size_t i = 0; i < p_program->func_cnt; ++i) {
        p_mir_func p_func = p_program->func_table + i;
        mir_func_print(p_func);
    }
    printf(" === mir program end ===\n");
}