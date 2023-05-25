#include <stdio.h>
#include <symbol_print.h>

#include <hir_print.h>
#include <symbol/var.h>
#include <symbol/type.h>

void symbol_name_print(p_symbol_var p_var) {
    if (p_var->is_global) {
        printf("@%s", p_var->name);
        return;
    }
    printf("%%%s", p_var->name);
}

void symbol_init_print(p_symbol_var p_var) {
    symbol_name_print(p_var);
    printf(" -> ");
    symbol_type_print(p_var->p_type);
    if (!p_var->is_def) {
        printf("\n");
        return;
    }

    assert(p_var->p_init);
    printf(" = {");
    for (size_t i = 0; i < p_var->p_init->size; ++i) {
        if (i > 0) printf(", ");
        if (p_var->p_init->basic == type_int)
            printf("%ld", p_var->p_init->memory[i].i);
        else
            printf("%lf", p_var->p_init->memory[i].f);
    }
    printf("}\n");
}
