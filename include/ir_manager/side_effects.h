#ifndef __IR_MANAGER_SIDE_EFFECTS__
#define __IR_MANAGER_SIDE_EFFECTS__

#include <program/use.h>
#include <symbol.h>

typedef struct func_side_effects func_side_effects, *p_func_side_effects;
typedef struct mem_visit_node mem_visit_node, *p_mem_visit_node;

struct func_side_effects {
    p_symbol_func p_func;
    list_head stored_global, loaded_global;

    bool *stored_param, *loaded_param;
    size_t param_cnt, loaded_param_cnt, stored_param_cnt;

    bool input, output;

    bool pure;
};

struct mem_visit_node {
    p_symbol_var p_global;
    list_head node;
};

void ir_side_effects_print(p_program p_ir);
void ir_side_effects(p_program p_ir);
void ir_side_effects_drop(p_symbol_func p_func);

#endif
