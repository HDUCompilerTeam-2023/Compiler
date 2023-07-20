#include <program/def.h>
#include <program/gen.h>
#include <program/use.h>

#include <ir_gen.h>
#include <symbol_gen.h>

p_program program_gen(const char *input, const char *output) {
    p_program p_program = malloc(sizeof(*p_program));
    *p_program = (program) {
        .variable = list_head_init(&p_program->variable),
        .constant = list_head_init(&p_program->constant),
        .function = list_head_init(&p_program->function),
        .string = list_head_init(&p_program->string),
        .variable_cnt = 0,
        .constant_cnt = 0,
        .function_cnt = 0,
        .input = NULL,
        .output = NULL,
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
    program_nestedtree_drop(p_program);
    while (!list_head_alone(&p_program->function)) {
        p_symbol_func p_del = list_entry(p_program->function.p_next, symbol_func, node);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&p_program->variable)) {
        p_symbol_var p_del = list_entry(p_program->variable.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_program->constant)) {
        p_symbol_var p_del = list_entry(p_program->constant.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&p_program->string)) {
        p_symbol_str p_del = list_entry(p_program->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
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
    return list_add_prev(&p_var->node, &p_program->variable);
}

bool program_add_constant(p_program p_program, p_symbol_var p_var) {
    p_var->id = p_program->constant_cnt++;
    return list_add_prev(&p_var->node, &p_program->constant);
}

bool program_add_function(p_program p_program, p_symbol_func p_func) {
    p_func->id = p_program->function_cnt++;
    return list_add_prev(&p_func->node, &p_program->function);
}
