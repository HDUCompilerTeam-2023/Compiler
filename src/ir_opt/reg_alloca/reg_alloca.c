#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/reg_alloca/whole_in_mem.h>
#include <ir_opt/reg_alloca/graph_alloca/graph_alloca.h>

#include <program/def.h>
#include <symbol/func.h>

void reg_alloca_pass(alloca_type type, size_t reg_r_num, size_t reg_s_num, p_program program) {
    p_list_head p_node;
    list_for_each(p_node, &program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        switch (type) {
        case alloca_color_graph:
            graph_alloca(p_func, reg_r_num, reg_s_num);
            break;
        case alloca_whole_in_mem:
            whole_in_mem_alloca(p_func, reg_r_num, reg_s_num);
            break;
        }
    }
}