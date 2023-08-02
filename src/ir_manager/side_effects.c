#include <ir_manager/side_effects.h>
#include <symbol_gen/func.h>
#include <ir/vreg.h>
#include <ir/instr.h>
#include <ir/param.h>
#include <ir/bb_param.h>
#include <ir/basic_block.h>
#include <ir/operand.h>
#include <symbol/var.h>
#include <symbol/type.h>

static inline void _fse_gen(p_symbol_func p_func) {
    p_func_side_effects p_side_effects = malloc(sizeof(*p_side_effects));
    *p_side_effects = (func_side_effects) {
        .p_func = p_func,
        .loaded_global = list_init_head(&p_side_effects->loaded_global),
        .stored_global = list_init_head(&p_side_effects->stored_global),
        .param_cnt = p_func->param_reg_cnt,
        .loaded_param_cnt = 0,
        .stored_param_cnt = 0,
        .loaded_param = malloc(sizeof(bool) * p_func->param_reg_cnt),
        .stored_param = malloc(sizeof(bool) * p_func->param_reg_cnt),
        .pure = true,
    };
    p_func->p_side_effects = p_side_effects;
    memset(p_side_effects->loaded_param, 0, sizeof(bool) * p_side_effects->param_cnt);
    memset(p_side_effects->stored_param, 0, sizeof(bool) * p_side_effects->param_cnt);
}
static inline void _fse_drop(p_symbol_func p_func) {
    assert(p_func->p_side_effects);
    p_func_side_effects p_side_effects = p_func->p_side_effects;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_side_effects->loaded_global) {
        p_mem_visit_node p_mv_node = list_entry(p_node, mem_visit_node, node);
        list_del(&p_mv_node->node);
        free(p_mv_node);
    }
    list_for_each_safe(p_node, p_next, &p_side_effects->stored_global) {
        p_mem_visit_node p_mv_node = list_entry(p_node, mem_visit_node, node);
        list_del(&p_mv_node->node);
        free(p_mv_node);
    }
    free(p_side_effects->loaded_param);
    free(p_side_effects->stored_param);
    free(p_side_effects);
    p_func->p_side_effects = NULL;
}

static inline bool _fse_add_global(p_symbol_func p_func, p_symbol_var p_global, bool is_store) {
    p_func->p_side_effects->pure = false;
    p_list_head p_head;
    if (is_store)
        p_head = &p_func->p_side_effects->stored_global;
    else
        p_head = &p_func->p_side_effects->loaded_global;
    p_list_head p_node;
    list_for_each(p_node, p_head) {
        p_mem_visit_node p_visit = list_entry(p_node, mem_visit_node, node);
        if (p_visit->p_global == p_global)
            return false;
    }
    p_mem_visit_node p_visit = malloc(sizeof(*p_visit));
    *p_visit = (mem_visit_node) {
        .p_global = p_global,
        .node = list_init_head(&p_visit->node),
    };
    list_add_prev(&p_visit->node, p_head);
    return true;
}

static inline bool _fse_add_param(p_symbol_func p_func, size_t param_id, bool is_store) {
    p_func->p_side_effects->pure = false;
    bool *p_head;
    if (is_store)
        p_head = p_func->p_side_effects->stored_param;
    else
        p_head = p_func->p_side_effects->loaded_param;
    if (p_head[param_id])
        return false;
    p_head[param_id] = true;
    if (is_store) {
        printf("func %s store %%%ld\n", p_func->name, param_id);
        ++p_func->p_side_effects->stored_param_cnt;
    }
    else {
        printf("func %s load %%%ld\n", p_func->name, param_id);
        ++p_func->p_side_effects->loaded_param_cnt;
    }
    return true;
}

static inline bool _fse_add_input(p_symbol_func p_func) {
    p_func->p_side_effects->pure = false;
    if(p_func->p_side_effects->input)
        return false;
    p_func->p_side_effects->input = true;
    return true;
}

static inline bool _fse_add_output(p_symbol_func p_func) {
    p_func->p_side_effects->pure = false;
    if(p_func->p_side_effects->output)
        return false;
    p_func->p_side_effects->output = true;
    return true;
}

typedef struct {
    p_call_graph_edge p_cg_edge;
    list_head node;
} fse_wl_node, *p_fse_wl_node;

static inline void _add_wl_node(p_symbol_func p_func, p_list_head p_head) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_call_graph_node->caller) {
        p_call_graph_edge p_cg_edge = list_entry(p_node, call_graph_edge, callee_node);
        assert(p_cg_edge->p_callee->p_func == p_func);
        printf("add edge %s -> %s to WL\n", p_cg_edge->p_caller->p_func->name, p_cg_edge->p_callee->p_func->name);
        p_fse_wl_node p_wl_node = malloc(sizeof(*p_wl_node));
        *p_wl_node = (fse_wl_node) {
            .p_cg_edge = p_cg_edge,
            .node = list_init_head(&p_wl_node->node),
        };
        list_add_prev(&p_wl_node->node, p_head);
    }
}

static inline p_call_graph_edge _pop_wl_node(p_list_head p_head) {
    if (list_head_alone(p_head))
        return NULL;
    p_fse_wl_node p_wl_node = list_entry(p_head->p_next, fse_wl_node, node);
    p_call_graph_edge p_ret = p_wl_node->p_cg_edge;
    list_del(&p_wl_node->node);
    free(p_wl_node);
    return p_ret;
}

// 0b00 -> none
// 0b01 -> load
// 0b10 -> store
// 0b11 -> both
#define NONE_FLAG  0
#define LOAD_FLAG  1
#define STORE_FLAG 2

typedef struct {
    enum {
        param_ref, global_ref, not_ref,
    } kind;
    union {
        size_t param_id;
        p_symbol_var p_global;
    };
    p_ir_vreg p_vreg;
} ref_info, *p_ref_info;

typedef struct {
    p_symbol_func p_func;
    p_ref_info reg_table;
    bool has_effects;
} func_ref_info, *p_func_ref_info;

static inline size_t _ref_side_effects(p_ir_operand p_ref, p_ref_info p_table) {
    assert(p_ref->kind  != not_ref);
    assert(p_ref->p_vreg);

    size_t flag = NONE_FLAG;

    p_ir_vreg p_des = NULL;
    p_ref_info p_des_info = NULL;
    switch (p_ref->used_type) {
    case instr_ptr:
        switch (p_ref->p_instr->irkind) {
        case ir_binary:
            assert(p_ref->p_instr->ir_binary.op == ir_add_op || p_ref->p_instr->ir_binary.op == ir_sub_op);
            p_des = p_ref->p_instr->ir_binary.p_des;
            p_des_info = p_table + p_des->id;
            break;
        case ir_unary:
            assert(p_ref->p_instr->ir_unary.op == ir_val_assign || p_ref->p_instr->ir_unary.op == ir_ptr_add_sp);
            p_des = p_ref->p_instr->ir_gep.p_des;
            p_des_info = p_table + p_des->id;
            break;
        case ir_gep:
            p_des = p_ref->p_instr->ir_gep.p_des;
            p_des_info = p_table + p_des->id;
            break;
        case ir_call:
            return NONE_FLAG;
        case ir_load:
            return LOAD_FLAG;
        case ir_store:
            assert(p_ref == p_ref->p_instr->ir_store.p_addr);
            return STORE_FLAG;
        }
        break;
    case bb_param_ptr:
        // TODO
        assert(0);
    case cond_ptr:
    case ret_ptr:
        assert(0);
    }

    assert(p_des);
    assert(p_des_info->kind == not_ref);
    assert(p_des_info->p_vreg == p_des);

    if (p_ref->kind == imme) {
        assert(p_ref->p_type->ref_level);
        assert(p_ref->p_vmem->is_global);
        p_des_info->kind = global_ref;
        p_des_info->p_global = p_ref->p_vmem;
    }
    else {
        p_ref_info p_src_info = p_table + p_ref->p_vreg->id;
        assert(p_src_info->kind != not_ref);
        assert(p_src_info->p_vreg == p_ref->p_vreg);
        p_des_info->kind = p_src_info->kind;
        if (p_src_info->kind == global_ref) {
            p_des_info->p_global = p_src_info->p_global;
        }
        else {
            p_des_info->param_id = p_src_info->param_id;
        }
    }

    p_list_head p_node;
    list_for_each(p_node, &p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        assert(p_use->kind == reg);
        assert(p_use->p_vreg == p_des);
        flag |= _ref_side_effects(p_use, p_table);
    }

    return flag;
}

static inline void _func_side_effects_print(p_symbol_func p_func) {
    printf("- %s", p_func->name);
    if (p_func->p_side_effects->pure) {
        printf(" pure\n\n");
        assert(!p_func->p_side_effects->input);
        assert(!p_func->p_side_effects->output);
        assert(list_head_alone(&p_func->p_side_effects->loaded_global) && list_head_alone(&p_func->p_side_effects->stored_global));
        assert(!p_func->p_side_effects->loaded_param_cnt);
        assert(!p_func->p_side_effects->stored_param_cnt);
        return;
    }
    if (p_func->p_side_effects->input)
        printf(" stdin");
    if (p_func->p_side_effects->output)
        printf(" stdout");
    printf("\n");

    if (!list_head_alone(&p_func->p_side_effects->loaded_global) || !list_head_alone(&p_func->p_side_effects->stored_global))
        printf("  - global\n");
    p_list_head p_node;
    list_for_each(p_node, &p_func->p_side_effects->loaded_global) {
        p_mem_visit_node p_visit = list_entry(p_node, mem_visit_node, node);
        printf("    - load %s\n", p_visit->p_global->name);
    }
    list_for_each(p_node, &p_func->p_side_effects->stored_global) {
        p_mem_visit_node p_visit = list_entry(p_node, mem_visit_node, node);
        printf("    - store %s\n", p_visit->p_global->name);
    }

    if (p_func->p_side_effects->loaded_param_cnt ||
            p_func->p_side_effects->stored_param_cnt)
        printf("  - param\n");
    for (size_t i = 0; i < p_func->p_side_effects->param_cnt; ++i) {
        if (p_func->p_side_effects->loaded_param[i])
            printf("    - load param %%%ld\n", i);
        if (p_func->p_side_effects->stored_param[i])
            printf("    - store param %%%ld\n", i);
    }
    printf("\n");
}

void ir_side_effects_print(p_program p_ir) {
    p_list_head p_node;
    printf("extern function:\n");
    list_for_each(p_node, &p_ir->ext_function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        _func_side_effects_print(p_func);
    }
    printf("\n");

    printf("normal function:\n");
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        _func_side_effects_print(p_func);
    }
    printf("\n");
}

static inline void _side_effects_init_for_extern_func_param(p_symbol_func p_func, p_list_head p_head) {
    if (p_func->p_side_effects)
        _fse_drop(p_func);
    _fse_gen(p_func);
    if (!strcmp(p_func->name, "getint")) {
        _fse_add_input(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "getch")) {
        _fse_add_input(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "getfloat")) {
        _fse_add_input(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "getarray")) {
        _fse_add_input(p_func);
        _fse_add_param(p_func, 0, true);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "getfarray")) {
        _fse_add_input(p_func);
        _fse_add_param(p_func, 0, true);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putint")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putch")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putfloat")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putarray")) {
        _fse_add_output(p_func);
        _fse_add_param(p_func, 1, false);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putfarray")) {
        _fse_add_output(p_func);
        _fse_add_param(p_func, 1, false);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "putf")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "_sysy_starttime")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "_sysy_stoptime")) {
        _fse_add_output(p_func);
        _add_wl_node(p_func, p_head);
        return;
    }
    if (!strcmp(p_func->name, "memset")) {
        _fse_add_param(p_func, 0, true);
        _add_wl_node(p_func, p_head);
        return;
    }
    // no other extern function
    printf("%s\n", p_func->name);
    assert(0);
}

static inline bool _side_effects_init_for_normal_func_param(p_symbol_func p_func, p_ref_info reg_table, p_list_head p_head) {
    if (p_func->p_side_effects)
        _fse_drop(p_func);
    _fse_gen(p_func);
    symbol_func_set_block_id(p_func);
    bool has_effects = false;
    p_list_head p_node;
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_ref_info p_info = reg_table + p_vreg->id;
        p_info->p_vreg = p_vreg;
        p_info->kind = not_ref;
        p_info->param_id = 0;
    }
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_ref_info p_info = reg_table + p_vreg->id;
        p_info->p_vreg = p_vreg;
        if (!p_vreg->p_type->ref_level) {
            p_info->kind = not_ref;
            p_info->param_id = 0;
            continue;
        }
        assert(p_vreg->p_type->ref_level == 1);
        p_info->kind = param_ref;
        p_info->param_id = p_vreg->id;

        size_t flag = NONE_FLAG;
        p_list_head p_node;
        list_for_each(p_node, &p_vreg->use_list) {
            p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
            assert(p_use->kind == reg);
            assert(p_use->p_vreg == p_vreg);
            flag |= _ref_side_effects(p_use, reg_table);
        }

        if (flag & LOAD_FLAG) {
            has_effects = true;
            _fse_add_param(p_func, p_vreg->id, false);
        }
        if (flag & STORE_FLAG) {
            has_effects = true;
            _fse_add_param(p_func, p_vreg->id, true);
        }
    }
    return has_effects;
}

static inline p_symbol_func _find_func_for_symbol_var_ref(p_ir_operand p_ref) {
    p_symbol_func p_func;
    switch (p_ref->used_type) {
    case instr_ptr:
        p_func = p_ref->p_instr->p_basic_block->p_func;
        break;
    case bb_param_ptr:
        p_func = p_ref->p_bb_param->p_target->p_source_block->p_func;
        break;
    case cond_ptr:
    case ret_ptr:
        p_func = p_ref->p_basic_block->p_func;
        break;
    }
    return p_func;
}

static inline void _side_effects_init_for_variable(p_symbol_var p_var, p_func_ref_info func_ref_table) {
    p_list_head p_node;
    list_for_each(p_node, &p_var->ref_list) {
        p_ir_operand p_ref = list_entry(p_node, ir_operand, ref_node);
        assert(p_ref->kind == imme);
        assert(p_ref->p_type->ref_level);
        assert(p_ref->p_vmem == p_var);
        p_symbol_func p_func = _find_func_for_symbol_var_ref(p_ref);
        p_ref_info reg_table = func_ref_table[p_func->id].reg_table;
        size_t flag = _ref_side_effects(p_ref, reg_table);
        if (flag & LOAD_FLAG) {
            func_ref_table[p_func->id].has_effects = true;
            _fse_add_global(p_func, p_var, false);
        }
        if (flag & STORE_FLAG) {
            func_ref_table[p_func->id].has_effects = true;
            _fse_add_global(p_func, p_var, true);
        }
    }
}

void ir_side_effects(p_program p_ir) {
    list_head wl_head = list_init_head(&wl_head);

    p_list_head p_node;
    list_for_each(p_node, &p_ir->ext_function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        _side_effects_init_for_extern_func_param(p_func, &wl_head);
    }

    p_func_ref_info func_ref_table = malloc(sizeof(*func_ref_table) * p_ir->function_cnt);
    size_t func_id = 0;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        size_t vreg_cnt = p_func->vreg_cnt + p_func->param_reg_cnt;
        p_ref_info reg_table = malloc(sizeof(*reg_table) * vreg_cnt);
        bool has_effects = _side_effects_init_for_normal_func_param(p_func, reg_table, &wl_head);
        func_ref_table[func_id].p_func = p_func;
        func_ref_table[func_id].reg_table = reg_table;
        func_ref_table[func_id].has_effects = has_effects;
        p_func->id = func_id++;
    }

    list_for_each(p_node, &p_ir->variable) {
        p_symbol_var p_global = list_entry(p_node, symbol_var, node);
        assert(p_global->is_global);
        assert(p_global->p_program == p_ir);
        _side_effects_init_for_variable(p_global, func_ref_table);
    }

    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (func_ref_table[p_func->id].has_effects)
            _add_wl_node(p_func, &wl_head);
    }

    p_call_graph_edge p_cg_edge;
    while (p_cg_edge = _pop_wl_node(&wl_head), p_cg_edge) {
        p_symbol_func p_caller = p_cg_edge->p_caller->p_func;
        p_symbol_func p_callee = p_cg_edge->p_callee->p_func;
        if (p_caller == p_callee)
            continue;

        printf("deal wl %s -> %s\n", p_caller->name, p_callee->name);
        bool has_change = false;

        if (p_callee->p_side_effects->input)
            has_change |= _fse_add_input(p_caller);
        if (p_callee->p_side_effects->output)
            has_change |= _fse_add_output(p_caller);

        p_list_head p_node;
        list_for_each(p_node, &p_callee->p_side_effects->loaded_global) {
            p_mem_visit_node p_visit = list_entry(p_node, mem_visit_node, node);
            has_change |= _fse_add_global(p_caller, p_visit->p_global, false);
        }
        list_for_each(p_node, &p_callee->p_side_effects->stored_global) {
            p_mem_visit_node p_visit = list_entry(p_node, mem_visit_node, node);
            has_change |= _fse_add_global(p_caller, p_visit->p_global, true);
        }

        if (p_callee->p_side_effects->loaded_param_cnt ||
                p_callee->p_side_effects->stored_param_cnt) {
            list_for_each(p_node, &p_cg_edge->call_instr) {
                p_call_instr_node p_ci_node = list_entry(p_node, call_instr_node, node);
                p_ir_instr p_call = p_ci_node->p_call_instr;
                p_list_head p_node;
                size_t param_id = 0;
                list_for_each(p_node, &p_call->ir_call.param_list) {
                    bool has_load = false, has_store = false;
                    if (p_callee->p_side_effects->loaded_param[param_id])
                        has_load = true;
                    if (p_callee->p_side_effects->stored_param[param_id])
                        has_store = true;
                    ++param_id;
                    if (!has_store && !has_load) {
                        continue;
                    }

                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    assert(p_param->p_type->ref_level);

                    if (p_param->kind == reg) {
                        p_ref_info p_info = func_ref_table[p_caller->id].reg_table + p_param->p_vreg->id;
                        if (p_info->kind == not_ref)
                            continue;
                        if (p_info->kind == global_ref) {
                            if (has_load)
                                has_change |= _fse_add_global(p_caller, p_info->p_global, false);
                            if (has_store)
                                has_change |= _fse_add_global(p_caller, p_info->p_global, true);
                        }
                        else {
                            if (has_load)
                                has_change |= _fse_add_param(p_caller, p_info->param_id, false);
                            if (has_store)
                                has_change |= _fse_add_param(p_caller, p_info->param_id, true);
                        }
                    }
                    else {
                        if (!p_param->p_vmem->is_global)
                            continue;
                        if (has_load)
                            has_change |= _fse_add_global(p_caller, p_param->p_vmem, false);
                        if (has_store)
                            has_change |= _fse_add_global(p_caller, p_param->p_vmem, true);
                    }
                }
            }
        }

        if (has_change)
            _add_wl_node(p_caller, &wl_head);
    }

    ir_side_effects_print(p_ir);

    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        size_t vreg_cnt = p_func->vreg_cnt + p_func->param_reg_cnt;
        p_ref_info reg_table = func_ref_table[p_func->id].reg_table;
        printf("reg ref table of %s:\n", p_func->name);
        for (size_t i = 0; i < vreg_cnt; ++i) {
            printf("%%%ld ", reg_table[i].p_vreg->id);
            if (reg_table[i].kind == not_ref) printf("not ref\n");
            else if (reg_table[i].kind == param_ref) printf("ref param %%%ld\n", reg_table[i].param_id);
            else if (reg_table[i].kind == global_ref) printf("ref global %s\n", reg_table[i].p_global->name);
        }
        free(reg_table);
    }
    free(func_ref_table);
}

void ir_side_effects_drop(p_symbol_func p_func) {
    if (!p_func->p_side_effects)
        return;
    _fse_drop(p_func);
    p_func->p_side_effects = NULL;
}
