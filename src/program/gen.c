#include <program/use.h>
#include <program/def.h>

#include <symbol_gen.h>
#include <hir_gen.h>
#include <mir_gen.h>

p_program program_gen(void) {
    p_program p_program = malloc(sizeof(*p_program));
    *p_program = (program) {
        .variable = list_head_init(&p_program->variable),
        .v_memory = list_head_init(&p_program->v_memory),
        .function = list_head_init(&p_program->function),
        .string = list_head_init(&p_program->string),
        .variable_cnt = 0,
        .v_memory_cnt = 0,
        .function_cnt = 0,
    };
    return p_program;
}
void program_hir_drop(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_h_func)
            hir_func_drop(p_sym->p_h_func);
        p_sym->p_h_func = NULL;
    }
}
void program_mir_drop(p_program p_program) {
    p_list_head p_node, p_netx;
    list_for_each_safe(p_node, p_netx, &p_program->v_memory) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        list_del(&p_vmem->node);
        mir_vmem_drop(p_vmem);
    }
    list_for_each(p_node, &p_program->function) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        if (p_sym->p_m_func)
            mir_func_drop(p_sym->p_m_func);
        p_sym->p_m_func = NULL;
    }
}
void program_drop(p_program p_program) {
    while (!list_head_alone(&p_program->function)) {
        p_symbol_sym p_del = list_entry(p_program->function.p_next, symbol_sym, node);
        assert(!p_del->p_h_func);
        assert(!p_del->p_m_func);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&p_program->variable)) {
        p_symbol_sym p_del = list_entry(p_program->variable.p_next, symbol_sym, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_program->string)) {
        p_symbol_str p_del = list_entry(p_program->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
    }

    free(p_program);
}

bool program_add_str(p_program p_program, p_symbol_str p_str) {
    return list_add_prev(&p_str->node, &p_program->string);
}

bool program_add_global(p_program p_program, p_symbol_sym p_sym) {
    p_sym->id = p_program->variable_cnt++;
    p_sym->is_global = true;
    return list_add_prev(&p_sym->node, &p_program->variable);
}
bool program_add_local(p_program p_program, p_symbol_sym p_sym) {
    assert(!list_head_alone(&p_program->function));
    p_symbol_sym p_func = list_entry(p_program->function.p_prev, symbol_sym, node);

    p_sym->id = p_func->variable_cnt++;
    p_sym->is_global = false;
    return list_add_prev(&p_sym->node, &p_func->variable);
}

bool program_add_function(p_program p_program, p_symbol_sym p_sym) {
    p_sym->id = p_program->function_cnt++;
    return list_add_prev(&p_sym->node, &p_program->function);
}

void program_mir_vmem_add(p_program p_program, p_mir_vmem p_vmem) {
    list_add_prev(&p_vmem->node, &p_program->v_memory);
    ++p_program->v_memory_cnt;
}
void program_mir_set_vmem_id(p_program p_program) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_program->v_memory) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        p_vmem->id = id++;
    }
}
