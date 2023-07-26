#include <ir_print.h>
#include <program/use.h>
#include <program/gen.h>
#include <symbol_gen/func.h>
#include <symbol_gen/var.h>
#include <symbol/type.h>
#include <ir/basic_block.h>
#include <ir_gen/instr.h>
#include <ir/operand.h>
#include <ir/vreg.h>
#include <ir/param.h>
#include <ir_opt/deadcode_elimate.h>

typedef enum {
    load_visit,
    store_visit,
    param_visit,
} mem_visit_kind;

typedef struct {
    p_ir_instr p_instr;
    list_head node;
} mem_visit_instr_node, *p_mem_visit_instr_node;

typedef struct {
    p_symbol_var p_var;
    bool has_load, has_store;
    list_head instr_list;
} mem_info, *p_mem_info;

static inline p_mem_visit_instr_node _instr_node_gen(p_ir_instr p_instr) {
    p_mem_visit_instr_node p_instr_node = malloc(sizeof(*p_instr_node));
    *p_instr_node = (mem_visit_instr_node) {
        .p_instr = p_instr,
        .node = list_init_head(&p_instr_node->node),
    };
    return p_instr_node;
}

// SysY 不存在别名, 在溢出之前处理，指针只会用于地址计算、load、store、函数参数，且load、store的地址来自于同一个来源
static inline void _collect_ref_use(p_ir_operand p_ref, p_mem_info p_info) {
    p_ir_vreg p_des = NULL;
    switch (p_ref->used_type) {
    case instr_ptr:
        switch (p_ref->p_instr->irkind) {
        case ir_binary:
            switch (p_ref->p_instr->ir_binary.op) {
            case ir_add_op:
            case ir_sub_op:
                p_des = p_ref->p_instr->ir_binary.p_des;
                break;
            default:
                assert(0);
            }
            break;
        case ir_unary:
            assert(p_ref->p_instr->ir_unary.op == ir_val_assign || p_ref->p_instr->ir_unary.op == ir_ptr_add_sp);
            p_des = p_ref->p_instr->ir_unary.p_des;
            break;
        case ir_gep:
            p_des = p_ref->p_instr->ir_gep.p_des;
            break;
        case ir_call:
            p_info->has_load = true;
            p_info->has_store = true;
            return;
        case ir_load:
            p_info->has_load = true;
            list_add_prev(&_instr_node_gen(p_ref->p_instr)->node, &p_info->instr_list);
            return;
        case ir_store:
            assert(p_ref != p_ref->p_instr->ir_store.p_src);
            p_info->has_store = true;
            list_add_prev(&_instr_node_gen(p_ref->p_instr)->node, &p_info->instr_list);
            return;
        }
        break;
    case bb_param_ptr:
        p_info->has_load = true;
        p_info->has_store = true;
        return;
    case cond_ptr:
    case ret_ptr:
        assert(0);
    }
    assert(p_des);

    p_list_head p_node;
    list_for_each(p_node, &p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        _collect_ref_use(p_use, p_info);
    }
}

static inline void _global_opt_variable(p_symbol_var p_var, p_mem_info p_info) {
    p_list_head p_node;
    list_for_each(p_node, &p_var->ref_list) {
        p_ir_operand p_ref = list_entry(p_node, ir_operand, ref_node);
        assert(p_ref->kind == imme);
        assert(p_ref->p_type->ref_level > 0);
        assert(p_ref->p_vmem == p_var);

        _collect_ref_use(p_ref, p_info);
    }
}

static inline void _global_opt_var_list(p_list_head p_list) {
    p_list_head p_node;
    list_for_each(p_node, p_list) {
        p_mem_info p_info = malloc(sizeof(*p_info));
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_info->p_var = p_var;
        p_info->has_store = false;
        p_info->has_load = false;
        p_info->instr_list = list_init_head(&p_info->instr_list);
        _global_opt_variable(p_var, p_info);

        if (!p_info->has_load) {
            printf("%s hasent be load\n", p_var->name);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_info->instr_list) {
                p_mem_visit_instr_node p_instr_node = list_entry(p_node, mem_visit_instr_node, node);
                p_ir_instr p_instr = p_instr_node->p_instr;
                list_del(&p_instr_node->node);
                free(p_instr_node);
                ir_instr_drop(p_instr);
            }
        }
        else {
            printf("%s has be store and load\n", p_var->name);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_info->instr_list) {
                p_mem_visit_instr_node p_instr_node = list_entry(p_node, mem_visit_instr_node, node);
                list_del(&p_instr_node->node);
                free(p_instr_node);
            }
        }
        free(p_info);
    }
}

void ir_opt_globalopt(p_program p_ir) {
    _global_opt_var_list(&p_ir->variable);

    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (!p_func->p_entry_block)
            continue;
        _global_opt_var_list(&p_func->param);
        _global_opt_var_list(&p_func->variable);
    }
}
