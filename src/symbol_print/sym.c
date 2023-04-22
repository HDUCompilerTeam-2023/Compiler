#include <symbol_print.h>
#include <stdio.h>

#include <symbol/type.h>
#include <symbol/sym.h>
#include <hir_print.h>

void symbol_define_print(p_symbol_sym p_sym) {
    if (p_sym->p_type->kind >= type_func) {
        printf("\n");
        return;
    }
    if (!p_sym->is_def) {
        printf("\n");
        return;
    }

    assert(p_sym->p_init);
    printf(" = {");
    for (size_t i = 0; i < p_sym->p_init->size; ++i) {
        if (i > 0) printf(", ");
        if (p_sym->p_init->basic == type_int)
            printf("%ld", p_sym->p_init->memory[i].i);
        else
            printf("%lf", p_sym->p_init->memory[i].f);
    }
    printf("}\n");
}

void symbol_name_print(p_symbol_sym p_sym) {
    if (p_sym->p_type->kind >= type_func) {
        printf("@%s", p_sym->name);
        return;
    }
    if (p_sym->is_global) {
        printf("@%s", p_sym->name);
        return;
    }
    printf("%%%s", p_sym->name);
}

void symbol_init_print(p_symbol_sym p_sym) {
    symbol_name_print(p_sym);
    printf(" -> ");
    symbol_type_print(p_sym->p_type);
    symbol_define_print(p_sym);
}

void symbol_param_print(p_symbol_sym p_sym) {
    p_list_head p_node;
    p_symbol_type p_param_type = p_sym->p_type->p_params;
    if (p_param_type) {
        printf("param:\n");
    }
    list_for_each(p_node, &p_sym->variable) {
      if (!p_param_type)
        break;
      symbol_init_print(list_entry(p_node, symbol_sym, node));
      p_param_type = p_param_type->p_params;
    }
}
