#include <symbol_gen.h>

#include <ir_gen/basic_block.h>
#include <ir_gen/bb_param.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir_gen/param.h>
#include <ir_gen/vreg.h>

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
        .call_param_vmem_list = list_head_init(&p_func->call_param_vmem_list),
        .vreg_list = list_head_init(&p_func->vreg_list),
        .stack_size = 0,
        .inner_stack_size = 0,
        .instr_num = 0,
    };
    strcpy(p_func->name, name);
    return p_func;
}

void symbol_func_bb_add_tail(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    p_basic_block->p_func = p_func;
    if (p_basic_block->p_branch->kind == ir_ret_branch)
        p_func->p_ret_block = p_basic_block;
    if (list_head_alone(&p_func->block))
        p_func->p_entry_block = p_basic_block;
    list_add_prev(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
}
void symbol_func_bb_add_head(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    p_basic_block->p_func = p_func;
    p_func->p_entry_block = p_basic_block;
    if (p_basic_block->p_branch->kind == ir_ret_branch) {
        assert(list_head_alone(&p_func->block));
        p_func->p_ret_block = p_basic_block;
    }
    list_add_next(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
}
void symbol_func_bb_del(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    assert(p_basic_block->p_func == p_func);
    if (p_func->p_entry_block == p_basic_block) {
        if (p_basic_block->node.p_next == &p_func->block)
            p_func->p_entry_block = NULL;
        else
            p_func->p_entry_block = list_entry(p_basic_block->node.p_next, ir_basic_block, node);
    }
    if (p_func->p_ret_block == p_basic_block) {
        if (p_basic_block->node.p_prev == &p_func->block)
            p_func->p_ret_block = NULL;
        else {
            p_func->p_ret_block = list_entry(p_basic_block->node.p_prev, ir_basic_block, node);
            assert(p_func->p_ret_block->p_branch->kind == ir_ret_branch);
        }
    }
    ir_basic_block_drop(p_basic_block);
}

void symbol_func_param_reg_add(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->param_reg_list);
    p_vreg->def_type = func_param_def;
    p_vreg->p_func = p_func;
    ++p_func->param_reg_cnt;
}
void symbol_func_param_reg_del(p_symbol_func p_func, p_ir_vreg p_vreg) {
    assert(p_vreg->p_func == p_func);
    assert(p_vreg->def_type == func_param_def);
    ir_vreg_drop(p_vreg);
    --p_func->param_reg_cnt;
}
p_symbol_var symbol_func_param_reg_mem(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_del(&p_vreg->node);
    symbol_func_vreg_add(p_func, p_vreg);
    p_symbol_var p_vmem = symbol_temp_var_gen(symbol_type_copy(p_vreg->p_type));
    symbol_func_add_param(p_func, p_vmem);
    --p_func->param_reg_cnt;
    return p_vmem;
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
        list_for_each(p_param_node, &p_basic_block->basic_block_phis) {
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
void symbol_func_add_call_param_vmem(p_symbol_func p_func, p_symbol_var p_vmem) {
    p_vmem->id = p_func->var_cnt++;
    if (p_vmem->stack_offset + basic_type_get_size(p_vmem->p_type->basic) > p_func->inner_stack_size)
        p_func->inner_stack_size = p_vmem->stack_offset + basic_type_get_size(p_vmem->p_type->basic);
    list_add_prev(&p_vmem->node, &p_func->call_param_vmem_list);
}
void symbol_func_add_param(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->var_cnt++;
    list_add_prev(&p_var->node, &p_func->param);
}

void symbol_func_drop(p_symbol_func p_func) {
    if (!list_head_alone(&p_func->block)) {
        assert(p_func->p_ret_block->p_branch->kind == ir_ret_branch);
        assert(p_func->p_entry_block == list_entry(p_func->block.p_next, ir_basic_block, node));
        assert(p_func->p_ret_block == list_entry(p_func->block.p_prev, ir_basic_block, node));
    }
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
    while (!list_head_alone(&p_func->call_param_vmem_list)) {
        p_symbol_var p_del = list_entry(p_func->call_param_vmem_list.p_next, symbol_var, node);
        symbol_var_drop(p_del);
    }
    while (!list_head_alone(&p_func->block)) {
        p_ir_basic_block p_del = list_entry(p_func->block.p_next, ir_basic_block, node);
        symbol_func_bb_del(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param_reg_list)) {
        p_ir_vreg p_vreg = list_entry(p_func->param_reg_list.p_next, ir_vreg, node);
        assert(p_vreg->def_type == func_param_def);
        symbol_func_param_reg_del(p_func, p_vreg);
    }
    while (!list_head_alone(&p_func->vreg_list)) {
        p_ir_vreg p_vreg = list_entry(p_func->vreg_list.p_next, ir_vreg, node);
        symbol_func_vreg_del(p_func, p_vreg);
    }
    assert(p_func->block_cnt == 0);
    assert(p_func->instr_num == 0);
    assert(p_func->vreg_cnt == 0);
    assert(p_func->param_reg_cnt == 0);
    free(p_func->name);
    free(p_func);
}

void symbol_func_clear_varible(p_symbol_func p_func) {
    if (!p_func->var_cnt)
        return;
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
    list_for_each(p_node, &p_func->call_param_vmem_list) {
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
                p_src_list = &p_instr->ir_call.param_list;
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
    size_t cnt = p_func->var_cnt;
    for (id = 0; id < cnt; ++id) {
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
    list_for_each(p_node, &p_func->call_param_vmem_list) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_var->id = id++;
    }
    assert(id == p_func->var_cnt);
}
