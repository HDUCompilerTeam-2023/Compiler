#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/reg_alloca/whole_in_mem.h>
#include <ir_opt/reg_alloca/min_spill.h>
#include <ir_opt/reg_alloca/graph_alloca/graph_alloca.h>

#include <program/def.h>
#include <symbol/func.h>

void reg_alloca_pass(alloca_type type, size_t reg_r_num, size_t reg_s_num, p_program program) {
    p_list_head p_node;
    list_for_each(p_node, &program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        size_t before_instr_num = p_func->instr_num;
        p_conflict_graph p_graph;
        if (list_head_alone(&p_func->block)) continue;
        switch (type) {
        case alloca_min_spill:
            ir_min_spill_func(p_func);
            p_graph = ir_gen_conflict_graph(p_func, reg_r_num, reg_s_num);
            graph_alloca(p_func, p_graph);
            ir_combine(p_func, p_graph);
            break;
        case alloca_color_graph:
            p_graph = ir_gen_conflict_graph(p_func, reg_r_num, reg_s_num);
            ir_graph_spill(p_func, p_graph);
            graph_alloca(p_func, p_graph);
            ir_combine(p_func, p_graph);
            break;
        case alloca_whole_in_mem:
            whole_in_mem_alloca(p_func, reg_r_num, reg_s_num);
            break;
        }
        printf("before alloca instr num %ld\n", before_instr_num);
        printf("after alloca instr num %ld\n", p_func->instr_num);
    }
}
