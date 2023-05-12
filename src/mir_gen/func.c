#include <mir_gen.h>
#include <mir_gen/func.h>

p_mir_func mir_func_table_gen(size_t cnt) {
    p_mir_func p_func = malloc(sizeof(*p_func) * cnt);
    for (size_t i = 0; i < cnt; ++i) {
        p_func[i] = (mir_func) {
            .entry_block = list_head_init(&(p_func + i)->entry_block),
            .p_func_sym = NULL,
            .vreg_list = list_head_init(&(p_func + i)->vreg_list),
            .vmem_list = list_head_init(&(p_func + i)->vmem_list),
        };
    }
    return p_func;
}

void mir_func_add_basic_block(p_mir_func p_func, p_mir_basic_block p_basic_block) {
    list_add_prev(&p_basic_block->node, &p_func->entry_block);
}

void mir_func_vreg_add(p_mir_func p_func, p_mir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->vreg_list);
}

void mir_func_vmem_add(p_mir_func p_func, p_mir_vmem p_vmem) {
    list_add_prev(&p_vmem->node, &p_func->vmem_list);
}

void mir_func_vreg_add_at(p_mir_func p_func, p_mir_vreg p_new_sym, p_mir_basic_block p_current_block, p_mir_instr p_instr) {
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
    while (p_block_node != &p_func->entry_block) {
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
    list_for_each(p_node, &p_func->entry_block) {
        p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block, node);
        p_basic_block->block_id = id++;
    }
}

void mir_basic_block_init_visited(p_mir_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->entry_block)
        list_entry(p_node, mir_basic_block, node)
            ->if_visited
        = false;
}

void mir_func_set_vreg_id(p_mir_func p_func) {
    p_list_head p_node;
    size_t id = 0;
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

void mir_func_table_drop(p_mir_func p_func, size_t cnt) {
    for (size_t i = 0; i < cnt; ++i) {
        while (!list_head_alone(&(p_func + i)->entry_block)) {
            p_mir_basic_block p_del = list_entry((p_func + i)->entry_block.p_next, mir_basic_block, node);
            list_del(&p_del->node);
            mir_basic_block_drop(p_del);
        }
        while (!list_head_alone(&(p_func + i)->vreg_list)) {
            p_mir_vreg p_vreg = list_entry((p_func + i)->vreg_list.p_next, mir_vreg, node);
            list_del(&p_vreg->node);
            free(p_vreg);
        }
        while (!list_head_alone(&(p_func + i)->vmem_list)) {
            p_mir_vmem p_vmem = list_entry((p_func + i)->vmem_list.p_next, mir_vmem, node);
            list_del(&p_vmem->node);
            free(p_vmem);
        }
    }
    free(p_func);
}
