#include <symbol_gen.h>

p_symbol_func symbol_func_gen(const char *name, basic_type b_type, bool is_va) {
    p_symbol_func p_func = malloc(sizeof(*p_func));
    *p_func = (symbol_func) {
        .node = list_head_init(&p_func->node),
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
        .ret_type = b_type,
        .id = 0,
        .is_va = is_va,
        .param = list_head_init(&p_func->param),
        .param_cnt = 0,
        .constant = list_head_init(&p_func->constant),
        .constant_cnt = 0,
        .variable = list_head_init(&p_func->variable),
        .variable_cnt = 0,
        .p_h_block = NULL,
        .p_m_func = NULL,
    };
    strcpy(p_func->name, name);
    return p_func;
}

void symbol_func_add_constant(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->param_cnt + p_func->variable_cnt + p_func->constant_cnt++;
    list_add_prev(&p_var->node, &p_func->constant);
}
void symbol_func_add_variable(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->param_cnt + p_func->variable_cnt++ + p_func->constant_cnt;
    list_add_prev(&p_var->node, &p_func->variable);
}
void symbol_func_add_param(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->param_cnt++;
    list_add_prev(&p_var->node, &p_func->param);
}

void symbol_func_drop(p_symbol_func p_func) {
    list_del(&p_func->node);
    while (!list_head_alone(&p_func->param)) {
        p_symbol_var p_del = list_entry(p_func->param.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }
    while (!list_head_alone(&p_func->constant)) {
        p_symbol_var p_del = list_entry(p_func->constant.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }
    while (!list_head_alone(&p_func->variable)) {
        p_symbol_var p_del = list_entry(p_func->variable.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }
    free(p_func->name);
    free(p_func);
}
