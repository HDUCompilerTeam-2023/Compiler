#include <program/use.h>
#include <program/def.h>
#include <symbol_gen/var.h>
#include <symbol_gen/func.h>
#include <ir/param.h>
#include <ir/bb_param.h>
#include <ir_gen/vreg.h>
#include <ir_gen/operand.h>
#include <ir_gen/instr.h>
#include <ir_gen/basic_block.h>
#include <ir_manager/call_graph.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/code_copy.h>
#include <program/print.h>

typedef struct {
    p_ir_instr p_call;
    p_symbol_func p_caller, p_callee;
    list_head node;
} call_node, *p_call_node;

static inline void _add_call_node(p_list_head p_head, p_ir_instr p_call) {
    p_symbol_func p_caller = p_call->p_basic_block->p_func, p_callee = p_call->ir_call.p_func;
    p_call_node p_cn = malloc(sizeof(*p_cn));
    *p_cn = (call_node) {
        .p_call = p_call,
        .p_caller = p_caller,
        .p_callee = p_callee,
        .node = list_init_head(&p_cn->node),
    };
    list_add_prev(&p_cn->node, p_head);
}

static inline bool _inline_instr_check(p_ir_instr p_call) {
    p_symbol_func p_func = p_call->ir_call.p_func;
    p_call_graph_node p_cg_node = p_func->p_call_graph_node;

    size_t call_cnt = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_cg_node->caller) {
        p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, callee_node);
        call_cnt += p_cg_edge->call_instr_cnt;
    }

    if (call_cnt == 1)
        return true;

    if (p_func->instr_num < 120)
        return true;

    return false;
}

static inline void _choose_call_node(p_call_node p_cn) {
    if (_inline_instr_check(p_cn->p_call))
        return;

    list_del(&p_cn->node);
    free(p_cn);
}

static inline void _inline_call_node(p_call_node p_cn) {
    p_ir_instr p_call = p_cn->p_call;
    list_del(&p_cn->node);
    free(p_cn);

    p_symbol_func p_callee = p_call->ir_call.p_func;
    p_symbol_func p_caller = p_call->p_basic_block->p_func;

    p_ir_basic_block p_prev_block = ir_basic_block_gen();
    p_ir_basic_block p_next_block = p_call->p_basic_block;
    ir_basic_block_insert_prev(p_prev_block, p_next_block);
    if (p_next_block == p_caller->p_entry_block)
        p_caller->p_entry_block = p_prev_block;

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_next_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        assert(p_instr->p_basic_block == p_next_block);
        if (p_instr == p_call)
            break;
        ir_instr_del(p_instr);
        ir_basic_block_addinstr_tail(p_prev_block, p_instr);
    }
    list_for_each_safe(p_node, p_next, &p_next_block->prev_branch_target_list) {
        p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
        ir_basic_block_branch_del_prev_target(p_target);
        p_target->p_block = p_prev_block;
        ir_basic_block_add_prev_target(p_target, p_prev_block);
    }
    list_for_each_safe(p_node, p_next, &p_next_block->basic_block_phis) {
        p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
        p_ir_vreg p_vreg = p_phi->p_bb_phi;
        ir_basic_block_del_phi(p_next_block, p_phi);
        ir_basic_block_add_phi(p_prev_block, p_vreg);
    }

    p_copy_map p_map = ir_code_copy_map_gen();

    list_for_each(p_node, &p_callee->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_symbol_var p_var_copy = ir_code_copy_var(p_var, p_map);
        symbol_func_add_variable(p_caller, p_var_copy);
    }
    list_for_each(p_node, &p_callee->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_ir_vreg p_vreg_copy = ir_code_copy_vreg(p_vreg, p_map);
        symbol_func_vreg_add(p_caller, p_vreg_copy);
    }
    p_list_head p_node_src = p_call->ir_call.param_list.p_next;
    list_for_each(p_node, &p_callee->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_ir_vreg p_vreg_copy = ir_code_copy_vreg(p_vreg, p_map);
        symbol_func_vreg_add(p_caller, p_vreg_copy);

        assert(p_node_src != &p_call->ir_call.param_list);
        p_ir_param p_param = list_entry(p_node_src, ir_param, node);
        p_ir_instr p_param_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_copy(p_param->p_param), p_vreg_copy);
        ir_basic_block_addinstr_tail(p_prev_block, p_param_assign);
        p_node_src = p_node_src->p_next;
    }
    assert(p_node_src == &p_call->ir_call.param_list);
    list_for_each(p_node, &p_callee->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        p_ir_basic_block p_bb_copy = ir_code_copy_bb(p_bb, p_map);
        ir_basic_block_insert_prev(p_bb_copy, p_next_block);
        if (p_bb == p_callee->p_entry_block) ir_basic_block_set_br(p_prev_block, p_bb_copy);
    }
    list_for_each(p_node, &p_callee->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        ir_code_copy_instr_of_block_inline(p_bb, p_map, p_call, p_next_block);
    }

    ir_code_copy_map_drop(p_map);
}

void ir_opt_inline(p_program p_ir) {
    list_head head = list_init_head(&head);
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        p_call_graph_node p_cg_node = p_func->p_call_graph_node;

        if (p_cg_node->is_recursion)
            continue;

        p_list_head p_node;
        list_for_each(p_node, &p_cg_node->caller) {
            p_call_graph_edge p_edge = list_entry(p_node, call_graph_edge, callee_node);
            assert(p_edge->p_callee->p_func == p_func);
            p_symbol_func p_caller = p_edge->p_caller->p_func;

            p_list_head p_node;
            list_for_each(p_node, &p_edge->call_instr) {
                p_ir_instr p_call = list_entry(p_node, call_instr_node, node)->p_call_instr;
                assert(p_call->p_basic_block->p_func == p_caller);
                assert(p_call->ir_call.p_func == p_func);
                _add_call_node(&head, p_call);
            }
        }
    }

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &head) {
        p_call_node p_cn = list_entry(p_node, call_node, node);
        _choose_call_node(p_cn);
    }

    list_for_each_safe(p_node, p_next, &head) {
        p_call_node p_cn = list_entry(p_node, call_node, node);
        _inline_call_node(p_cn);
    }

    p_list_head p_prev;
    list_for_each_tail_safe(p_node, p_prev, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        p_call_graph_node p_cg_node = p_func->p_call_graph_node;
        size_t call_cnt = p_cg_node->caller_cnt;
        if (!strcmp(p_func->name, "main"))
            ++call_cnt;
        if (p_cg_node->is_recursion)
            --call_cnt;
        assert(call_cnt >= 0);
        if (!call_cnt) {
            symbol_func_drop(p_func);
            --p_ir->function_cnt;
        }
    }

    ir_deadcode_elimate_pass(p_ir, true);
}
