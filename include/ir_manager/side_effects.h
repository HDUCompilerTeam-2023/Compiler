#ifndef __IR_MANAGER_SIDE_EFFECTS__
#define __IR_MANAGER_SIDE_EFFECTS__

#include <program/use.h>
#include <symbol.h>
#include <ir/instr.h>

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

typedef struct {
    p_ir_instr p_instr;
    list_head node;
} mem_visit_instr_node, *p_mem_visit_instr_node;

typedef struct {
    p_symbol_var p_var;
    list_head load_list;
    list_head store_list;
} mem_info, *p_mem_info;

void ir_side_effects_print(p_program p_ir);
void ir_side_effects(p_program p_ir);
void ir_side_effects_drop(p_symbol_func p_func);
void mem_visit_info_drop(p_symbol_var p_var);

#endif
