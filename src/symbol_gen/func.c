#include <symbol_gen.h>

#include <ir_gen/basic_block.h>
#include <ir_gen/bb_param.h>
#include <ir_gen/instr.h>
#include <ir_gen/vreg.h>
#include <ir_gen/operand.h>
#include <ir_gen/param.h>

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

void symbol_func_bb_add(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    list_add_prev(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
}
void symbol_func_bb_del(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    list_del(&p_basic_block->node);
    ir_basic_block_drop(p_basic_block);
    --p_func->block_cnt;
}

void symbol_func_param_reg_add(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->param_reg_list);
    ++p_func->param_reg_cnt;
}
void symbol_func_param_reg_del(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_del(&p_vreg->node);
    ir_vreg_drop(p_vreg);
    --p_func->param_reg_cnt;
}

void symbol_func_vreg_add(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->vreg_list);
    ++p_func->vreg_cnt;
}
void symbol_func_vreg_del(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_del(&p_vreg->node);
    ir_vreg_drop(p_vreg);
    --p_func->vreg_cnt;
}

void symbol_func_set_block_id(p_symbol_func p_func) {
    size_t block_id = 0;
    size_t instr_id = 0;
    size_t vreg_id = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_vreg->id = vreg_id++;
    }
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_list_head p_param_node;
        list_for_each(p_param_node, &p_basic_block->basic_block_phis->bb_phi) {
            p_ir_bb_phi p_bb_phi = list_entry(p_param_node, ir_bb_phi, node);
            p_bb_phi->p_bb_phi->id = vreg_id++;
        }
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            p_instr->instr_id = instr_id++;
            p_ir_vreg p_des = ir_instr_get_des(p_instr);
            if (p_des) p_des->id = vreg_id++;
        }
        p_basic_block->block_id = block_id++;
    }
}

void symbol_func_basic_block_init_visited(p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->block)
        list_entry(p_node, ir_basic_block, node)
            ->if_visited
        = false;
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
        p_ir_basic_block p_del = list_entry(p_func->block.p_next, ir_basic_block, node);
        symbol_func_bb_del(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param_reg_list)) {
        p_ir_vreg p_vreg = list_entry(p_func->param_reg_list.p_next, ir_vreg, node);
        symbol_func_param_reg_del(p_func, p_vreg);
    }
    while (!list_head_alone(&p_func->vreg_list)) {
        p_ir_vreg p_vreg = list_entry(p_func->vreg_list.p_next, ir_vreg, node);
        symbol_func_vreg_del(p_func, p_vreg);
    }
    free(p_func->name);
    free(p_func);
}

void symbol_func_clear_varible(p_symbol_func p_func) {
    p_symbol_var *p_map = malloc(sizeof(void *) * p_func->var_cnt);
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_map[id] = p_var;
        p_var->id = id++;
    }
    list_for_each(p_node, &p_func->constant) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_map[id] = p_var;
        p_var->id = id++;
    }
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_map[id] = p_var;
        p_var->id = id++;
    }
    assert(id == p_func->var_cnt);
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_list_head p_instr_node;
        list_for_each(p_instr_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
            p_ir_operand p_src1 = NULL, p_src2 = NULL;
            p_list_head p_src_list = NULL;
            switch (p_instr->irkind) {
            case ir_binary:
                p_src1 = p_instr->ir_binary.p_src1;
                p_src2 = p_instr->ir_binary.p_src2;
                break;
            case ir_unary:
                p_src1 = p_instr->ir_unary.p_src;
                break;
            case ir_gep:
                p_src1 = p_instr->ir_gep.p_addr;
                p_src2 = p_instr->ir_gep.p_offset;
                break;
            case ir_load:
                p_src1 = p_instr->ir_load.p_addr;
                break;
            case ir_store:
                p_src1 = p_instr->ir_store.p_addr;
                p_src2 = p_instr->ir_store.p_src;
                break;
            case ir_call:
                p_src_list = &p_instr->ir_call.p_param_list->param;
                break;
            }
            if (p_src1 && p_src1->kind == imme && p_src1->p_type->ref_level > 0 && !p_src1->p_vmem->is_global) {
                assert(!p_map[p_src1->p_vmem->id] || p_map[p_src1->p_vmem->id] == p_src1->p_vmem);
                p_map[p_src1->p_vmem->id] = NULL;
            }
            if (p_src2 && p_src2->kind == imme && p_src2->p_type->ref_level > 0 && !p_src2->p_vmem->is_global) {
                assert(!p_map[p_src2->p_vmem->id] || p_map[p_src2->p_vmem->id] == p_src2->p_vmem);
                p_map[p_src2->p_vmem->id] = NULL;
            }
            p_list_head p_node;
            if (p_src_list) {
                list_for_each(p_node, p_src_list) {
                    p_ir_operand p_src = list_entry(p_node, ir_param, node)->p_param;
                    if (p_src->kind == imme && p_src->p_type->ref_level > 0 && !p_src->p_vmem->is_global) {
                        assert(!p_map[p_src->p_vmem->id] || p_map[p_src->p_vmem->id] == p_src->p_vmem);
                        p_map[p_src->p_vmem->id] = NULL;
                    }
                }
            }
        }
    }
    for(id = 0; id < p_func->var_cnt; ++id) {
        if (!p_map[id]) continue;
        symbol_func_delete_varible(p_func, p_map[id]);
    }
    free(p_map);
    symbol_func_set_varible_id(p_func);
}

void symbol_func_delete_varible(p_symbol_func p_func, p_symbol_var p_var) {
    list_del(&p_var->node);
    symbol_var_drop(p_var);
    --p_func->var_cnt;
}

void symbol_func_set_varible_id(p_symbol_func p_func) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_var->id = id++;
    }
    list_for_each(p_node, &p_func->constant) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_var->id = id++;
    }
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_var->id = id++;
    }
    assert(id == p_func->var_cnt);
}
