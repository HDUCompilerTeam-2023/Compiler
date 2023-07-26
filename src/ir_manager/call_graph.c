#include <program/use.h>
#include <program/def.h>
#include <symbol/func.h>
#include <ir_manager/call_graph.h>
#include <ir/basic_block.h>
#include <ir/instr.h>

static inline p_call_graph_edge _call_graph_edge_gen(p_call_graph_node p_caller, p_call_graph_node p_callee) {
    p_call_graph_edge p_cg_edge = malloc(sizeof(*p_cg_edge));
    *p_cg_edge = (call_graph_edge) {
        .p_caller = p_caller,
        .caller_node = list_init_head(&p_cg_edge->caller_node),
        .p_callee = p_callee,
        .callee_node = list_init_head(&p_cg_edge->callee_node),
        .call_instr_cnt = 0,
        .call_instr = list_init_head(&p_cg_edge->call_instr),
    };
    ++p_caller->callee_cnt;
    assert(list_add_prev(&p_cg_edge->caller_node, &p_caller->callee));
    ++p_callee->caller_cnt;
    assert(list_add_prev(&p_cg_edge->callee_node, &p_callee->caller));
    if (p_caller == p_callee) {
        assert(!p_callee->is_recursion);
        p_callee->is_recursion = true;
    }
    return p_cg_edge;
}

static inline void _call_graph_edge_drop(p_call_graph_edge p_cg_edge) {
    if (p_cg_edge->p_caller == p_cg_edge->p_callee) {
        assert(p_cg_edge->p_callee->is_recursion);
        p_cg_edge->p_callee->is_recursion = false;
    }
    --p_cg_edge->p_caller->callee_cnt;
    assert(list_del(&p_cg_edge->caller_node));
    --p_cg_edge->p_callee->caller_cnt;
    assert(list_del(&p_cg_edge->callee_node));
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_cg_edge->call_instr) {
        p_call_instr_node p_ci_node = list_entry(p_node, call_instr_node, node);
        list_del(&p_ci_node->node);
        p_ci_node->p_call_instr->ir_call.p_ci_node = NULL;
        free(p_ci_node);
    }
    free(p_cg_edge);
}

void ir_call_instr_node_gen(p_ir_instr p_instr) {
    assert(!p_instr->ir_call.p_ci_node);
    p_call_graph_node p_caller = p_instr->p_basic_block->p_func->p_call_graph_node;
    assert(p_caller);
    assert(p_caller->p_func == p_instr->p_basic_block->p_func);
    p_call_graph_node p_callee = p_instr->ir_call.p_func->p_call_graph_node;
    assert(p_callee);
    assert(p_callee->p_func == p_instr->ir_call.p_func);

    p_call_graph_edge p_edge = NULL;

    p_list_head p_node;
    list_for_each(p_node, &p_caller->callee) {
        p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, caller_node);
        if (p_cg_edge->p_callee == p_callee) {
            p_edge = p_cg_edge;
            break;
        }
    }
    if (!p_edge) {
        p_edge = _call_graph_edge_gen(p_caller, p_callee);
    }

    p_call_instr_node p_ci_node = malloc(sizeof(*p_ci_node));
    *p_ci_node = (call_instr_node) {
        .p_call_instr = p_instr,
        .p_cg_edge = p_edge,
        .node = list_init_head(&p_ci_node->node),
    };
    p_instr->ir_call.p_ci_node = p_ci_node;
    list_add_prev(&p_ci_node->node, &p_edge->call_instr);
    ++p_edge->call_instr_cnt;
}

void ir_call_instr_node_drop(p_ir_instr p_instr) {
    assert(p_instr);
    p_call_instr_node p_ci_node = p_instr->ir_call.p_ci_node;
    assert(p_ci_node);
    assert(p_ci_node->p_call_instr == p_instr);
    p_instr->ir_call.p_ci_node = NULL;

    --p_ci_node->p_cg_edge->call_instr_cnt;
    list_del(&p_ci_node->node);
    if (!p_ci_node->p_cg_edge->call_instr_cnt) {
        assert(list_head_alone(&p_ci_node->p_cg_edge->call_instr));
        _call_graph_edge_drop(p_ci_node->p_cg_edge);
    }

    free(p_ci_node);
}

void ir_call_graph_print(p_program p_ir) {
    printf("call graph:\n");
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        p_list_head p_node;
        printf("%s\n", p_func->name);
        if (p_func->p_call_graph_node->is_recursion)
            printf("* recursion\n");
        list_for_each(p_node, &p_func->p_call_graph_node->callee) {
            p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, caller_node);
            printf("  -> %s %ld\n", p_cg_edge->p_callee->p_func->name, p_cg_edge->call_instr_cnt);
            p_list_head p_node;
            list_for_each(p_node, &p_cg_edge->call_instr) {
                p_call_instr_node p_ci_node = list_entry(p_node, call_instr_node, node);
                printf("    at %ld\n", p_ci_node->p_call_instr->instr_id);
            }
        }
        printf("\n");
    }
}

void ir_build_call_graph(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        p_list_head p_node;
        list_for_each(p_node, &p_func->block) {
            p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
            p_list_head p_node;
            list_for_each(p_node, &p_bb->instr_list) {
                p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
                if (p_instr->irkind != ir_call)
                    continue;
                if (p_instr->ir_call.p_ci_node)
                    continue;
                assert(p_instr->p_basic_block->p_func == p_func);
                ir_call_instr_node_gen(p_instr);
            }
        }
    }
}

void ir_call_graph_node_gen(p_symbol_func p_func) {
    assert(p_func);
    assert(!p_func->p_call_graph_node);
    p_call_graph_node p_cg_node = malloc(sizeof(*p_cg_node));
    *p_cg_node = (call_graph_node) {
        .p_func = p_func,
        .callee_cnt = 0,
        .callee = list_init_head(&p_cg_node->callee),
        .caller_cnt = 0,
        .caller = list_init_head(&p_cg_node->caller),
        .is_recursion = false,
    };
    p_func->p_call_graph_node = p_cg_node;
}

void ir_call_graph_node_drop(p_symbol_func p_func) {
    assert(p_func);
    assert(p_func->p_call_graph_node);

    p_call_graph_node p_cg_node = p_func->p_call_graph_node;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_cg_node->callee) {
        p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, caller_node);
        _call_graph_edge_drop(p_cg_edge);
    }
    list_for_each_safe(p_node, p_next, &p_cg_node->caller) {
        p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, callee_node);
        _call_graph_edge_drop(p_cg_edge);
    }
    free(p_cg_node);
}
