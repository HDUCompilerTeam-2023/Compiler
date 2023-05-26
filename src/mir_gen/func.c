#include <mir_gen.h>
#include <mir_gen/func.h>

p_mir_func mir_func_gen() {
    p_mir_func p_func = malloc(sizeof(*p_func));
    *p_func = (mir_func) {
        .block = list_head_init(&p_func->block),
        .block_cnt = 0,
        .p_func_sym = NULL,
        .param_list = list_head_init(&p_func->param_list),
        .param_cnt = 0,
        .vreg_list = list_head_init(&p_func->vreg_list),
        .vreg_cnt = 0,
        .vmem_list = list_head_init(&p_func->vmem_list),
        .vmem_cnt = 0,
    };
    return p_func;
}

void mir_func_bb_add(p_mir_func p_func, p_mir_basic_block p_basic_block) {
    list_add_prev(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
}
void mir_func_bb_del(p_mir_func p_func, p_mir_basic_block p_basic_block) {
    list_del(&p_basic_block->node);
    mir_basic_block_drop(p_basic_block);
    --p_func->block_cnt;
}

void mir_func_param_add(p_mir_func p_func, p_mir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->param_list);
    ++p_func->param_cnt;
}
void mir_func_param_del(p_mir_func p_func, p_mir_vreg p_vreg) {
    list_del(&p_vreg->node);
    mir_vreg_drop(p_vreg);
    --p_func->param_cnt;
}

void mir_func_vreg_add(p_mir_func p_func, p_mir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->vreg_list);
    ++p_func->vreg_cnt;
}
void mir_func_vreg_del(p_mir_func p_func, p_mir_vreg p_vreg) {
    list_del(&p_vreg->node);
    mir_vreg_drop(p_vreg);
    --p_func->vreg_cnt;
}

void mir_func_vmem_add(p_mir_func p_func, p_mir_vmem p_vmem) {
    list_add_prev(&p_vmem->node, &p_func->vmem_list);
    ++p_func->vmem_cnt;
}
void mir_func_vmem_del(p_mir_func p_func, p_mir_vmem p_vmem) {
    list_del(&p_vmem->node);
    mir_vmem_drop(p_vmem);
    --p_func->vmem_cnt;
}

void mir_func_vreg_add_at(p_mir_func p_func, p_mir_vreg p_new_sym, p_mir_basic_block p_current_block, p_mir_instr p_instr) {
    ++p_func->vreg_cnt;
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
void mir_func_set_block_id(p_mir_func p_func) {
    size_t id = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block, node);
        p_basic_block->block_id = id++;
    }
}

void mir_basic_block_init_visited(p_mir_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->block)
        list_entry(p_node, mir_basic_block, node)
            ->if_visited
        = false;
}

void mir_func_set_vreg_id(p_mir_func p_func) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->param_list) {
        p_mir_vreg p_vreg = list_entry(p_node, mir_vreg, node);
        p_vreg->id = id++;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_mir_vreg p_vreg = list_entry(p_node, mir_vreg, node);
        p_vreg->id = id++;
    }
}
void mir_func_set_vmem_id(p_mir_func p_func) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->vmem_list) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        p_vmem->id = id++;
    }
}

void mir_func_drop(p_mir_func p_func) {
    while (!list_head_alone(&p_func->block)) {
        p_mir_basic_block p_del = list_entry(p_func->block.p_next, mir_basic_block, node);
        mir_func_bb_del(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param_list)) {
        p_mir_vreg p_vreg = list_entry(p_func->param_list.p_next, mir_vreg, node);
        mir_func_param_del(p_func, p_vreg);
    }
    while (!list_head_alone(&p_func->vreg_list)) {
        p_mir_vreg p_vreg = list_entry(p_func->vreg_list.p_next, mir_vreg, node);
        mir_func_vreg_del(p_func, p_vreg);
    }
    while (!list_head_alone(&p_func->vmem_list)) {
        p_mir_vmem p_vmem = list_entry(p_func->vmem_list.p_next, mir_vmem, node);
        mir_func_vmem_del(p_func, p_vmem);
    }
    free(p_func);
}
