#include <symbol_gen.h>

#include <mir_gen/basic_block.h>
#include <mir_gen/bb_param.h>
#include <mir_gen/instr.h>
#include <mir_gen/vreg.h>

p_symbol_func symbol_func_gen(const char *name, basic_type b_type, bool is_va) {
    p_symbol_func p_func = malloc(sizeof(*p_func));
    *p_func = (symbol_func) {
        .node = list_head_init(&p_func->node),
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
        .ret_type = b_type,
        .id = 0,
        .is_va = is_va,
        .var_cnt = 0,
        .param = list_head_init(&p_func->param),
        .constant = list_head_init(&p_func->constant),
        .variable = list_head_init(&p_func->variable),
        .block = list_head_init(&p_func->block),
        .block_cnt = 0,
        .param_reg_list = list_head_init(&p_func->param_reg_list),
        .vreg_list = list_head_init(&p_func->vreg_list),
    };
    strcpy(p_func->name, name);
    return p_func;
}

void symbol_func_bb_add(p_symbol_func p_func, p_mir_basic_block p_basic_block) {
    list_add_prev(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
}
void symbol_func_bb_del(p_symbol_func p_func, p_mir_basic_block p_basic_block) {
    list_del(&p_basic_block->node);
    mir_basic_block_drop(p_basic_block);
    --p_func->block_cnt;
}

void symbol_func_param_reg_add(p_symbol_func p_func, p_mir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->param_reg_list);
}
void symbol_func_param_reg_del(p_symbol_func p_func, p_mir_vreg p_vreg) {
    list_del(&p_vreg->node);
    mir_vreg_drop(p_vreg);
}

void symbol_func_vreg_add(p_symbol_func p_func, p_mir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->vreg_list);
}
void symbol_func_vreg_del(p_symbol_func p_func, p_mir_vreg p_vreg) {
    list_del(&p_vreg->node);
    mir_vreg_drop(p_vreg);
}

void symbol_func_vreg_add_at(p_symbol_func p_func, p_mir_vreg p_new_sym, p_mir_basic_block p_current_block, p_mir_instr p_instr) {
    p_list_head p_instr_node = p_instr->node.p_next;
    while (p_instr_node != &p_current_block->instr_list) {
        p_mir_instr p_instr = list_entry(p_instr_node, mir_instr, node);
        p_mir_vreg p_des = mir_instr_get_des(p_instr);
        if (p_des) {
            list_add_prev(&p_new_sym->node, &p_des->node);
            return;
        }
        p_instr_node = p_instr_node->p_next;
    }
    p_list_head p_block_node = p_current_block->node.p_next;
    while (p_block_node != &p_func->block) {
        p_mir_basic_block p_basic_block = list_entry(p_block_node, mir_basic_block, node);
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->basic_block_phis->bb_phi) {
            p_mir_vreg p_vreg = list_entry(p_node, mir_bb_phi, node)->p_bb_phi;
            list_add_prev(&p_new_sym->node, &p_vreg->node);
            return;
        }
        p_instr_node = p_basic_block->instr_list.p_next;
        while (p_instr_node != &p_basic_block->instr_list) {
            p_mir_instr p_instr = list_entry(p_instr_node, mir_instr, node);
            p_mir_vreg p_des = mir_instr_get_des(p_instr);
            if (p_des) {
                list_add_prev(&p_new_sym->node, &p_des->node);
                return;
            }
            p_instr_node = p_instr_node->p_next;
        }
        p_block_node = p_block_node->p_next;
    }
    list_add_prev(&p_new_sym->node, &p_func->vreg_list);
}

void symbol_func_set_block_id(p_symbol_func p_func) {
    size_t id = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block, node);
        p_basic_block->block_id = id++;
    }
}

void symbol_func_basic_block_init_visited(p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->block)
        list_entry(p_node, mir_basic_block, node)
            ->if_visited
        = false;
}

void symbol_func_set_vreg_id(p_symbol_func p_func) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_mir_vreg p_vreg = list_entry(p_node, mir_vreg, node);
        p_vreg->id = id++;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_mir_vreg p_vreg = list_entry(p_node, mir_vreg, node);
        p_vreg->id = id++;
    }
}

void symbol_func_add_constant(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->var_cnt++;
    list_add_prev(&p_var->node, &p_func->constant);
}
void symbol_func_add_variable(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->var_cnt++;
    list_add_prev(&p_var->node, &p_func->variable);
}
void symbol_func_add_param(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->var_cnt++;
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
    while (!list_head_alone(&p_func->block)) {
        p_mir_basic_block p_del = list_entry(p_func->block.p_next, mir_basic_block, node);
        symbol_func_bb_del(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param_reg_list)) {
        p_mir_vreg p_vreg = list_entry(p_func->param_reg_list.p_next, mir_vreg, node);
        symbol_func_param_reg_del(p_func, p_vreg);
    }
    while (!list_head_alone(&p_func->vreg_list)) {
        p_mir_vreg p_vreg = list_entry(p_func->vreg_list.p_next, mir_vreg, node);
        symbol_func_vreg_del(p_func, p_vreg);
    }
    free(p_func->name);
    free(p_func);
}
