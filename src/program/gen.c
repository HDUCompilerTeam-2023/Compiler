#include <program/def.h>
#include <program/gen.h>
#include <program/use.h>

#include <ir_gen.h>
#include <backend/arm/arm_struct_gen.h>
#include <symbol_gen.h>

p_program program_gen(const char *input, const char *output) {
    p_program p_program = malloc(sizeof(*p_program));
    *p_program = (program) {
        .variable = list_head_init(&p_program->variable),
        .function = list_head_init(&p_program->function),
        .ext_function = list_head_init(&p_program->ext_function),
        .string = list_head_init(&p_program->string),
        .variable_cnt = 0,
        .function_cnt = 0,
        .ext_function_cnt = 0,
        .input = NULL,
        .output = NULL,
        .arm_function = list_head_init(&p_program->arm_function),
    };
    if (!output) {
        char *tmp = malloc(sizeof(char) * 9);
        strcpy(tmp, "output.s");
        p_program->output = tmp;
    }
    else {
        char *tmp = malloc(sizeof(char) * (strlen(output) + 1));
        strcpy(tmp, output);
        p_program->output = tmp;
    }
    if (!input) {
        char *tmp = malloc(sizeof(char) * 10);
        strcpy(tmp, "std input");
        p_program->input = tmp;
    }
    else {
        char *tmp = malloc(sizeof(char) * (strlen(input) + 1));
        strcpy(tmp, input);
        p_program->input = tmp;
    }
    return p_program;
}
void program_drop(p_program p_program) {
    while (!list_head_alone(&p_program->function)) {
        p_symbol_func p_del = list_entry(p_program->function.p_next, symbol_func, node);
        assert(p_del->p_program == p_program);
        symbol_func_drop(p_del);
    }
    while (!list_head_alone(&p_program->ext_function)) {
        p_symbol_func p_del = list_entry(p_program->ext_function.p_next, symbol_func, node);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&p_program->variable)) {
        p_symbol_var p_del = list_entry(p_program->variable.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_program->string)) {
        p_symbol_str p_del = list_entry(p_program->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
    }
    while (!list_head_alone(&p_program->arm_function)) {
        p_arm_func p_del = list_entry(p_program->arm_function.p_next, arm_func, node);
        arm_func_drop(p_del);
    }
    free(p_program->input);
    free(p_program->output);
    free(p_program);
}

bool program_add_str(p_program p_program, p_symbol_str p_str) {
    return list_add_prev(&p_str->node, &p_program->string);
}

bool program_add_global(p_program p_program, p_symbol_var p_var) {
    p_var->id = p_program->variable_cnt++;
    assert(!p_var->p_func);
    assert(!p_var->p_program);
    p_var->p_program = p_program;
    p_var->is_global = true;
    return list_add_prev(&p_var->node, &p_program->variable);
}

bool program_add_function(p_program p_program, p_symbol_func p_func) {
    p_func->id = p_program->function_cnt++;
    p_func->p_program = p_program;
    return list_add_prev(&p_func->node, &p_program->function);
}

bool program_add_ext_function(p_program p_program, p_symbol_func p_func) {
    p_func->id = p_program->ext_function_cnt++;
    return list_add_prev(&p_func->node, &p_program->ext_function);
}

void program_global_set_id(p_program p_program) {
    size_t id = 0;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_program->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (list_head_alone(&p_var->ref_list)) {
            symbol_var_drop(p_var);
            continue;
        }
        p_var->id = id++;
    }
    assert(id == p_program->variable_cnt);
}
