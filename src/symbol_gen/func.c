#include <symbol_gen.h>

#include <ir_gen/basic_block.h>
#include <ir_gen/bb_param.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir_gen/param.h>
#include <ir_gen/varray.h>
#include <ir_gen/vreg.h>
#include <ir_manager/buildnestree.h>

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
        .variable = list_head_init(&p_func->variable),
        .block = list_head_init(&p_func->block),
        .block_cnt = 0,
        .param_reg_list = list_head_init(&p_func->param_reg_list),
        .p_nestedtree_root = NULL,
        .p_call_graph_node = NULL,
        .vreg_list = list_head_init(&p_func->vreg_list),
        .stack_size = 0,
        .instr_num = 0,
        .if_updated_graph = true,
        .param_vmem_base_num = 0,
        .param_vmem_base = list_head_init(&p_func->param_vmem_base),
        .varray_num = 0,
    };
    ir_call_graph_node_gen(p_func);
    strcpy(p_func->name, name);
    return p_func;
}
void symbol_func_param_vmem_base_add(p_symbol_func p_func, p_ir_param_vmem_base p_vmem_base) {
    p_func->varray_num += p_vmem_base->p_param_base->num;
    assert(list_add_prev(&p_vmem_base->node, &p_func->param_vmem_base));
}
p_ir_param_vmem_base symbol_func_get_param_vmem_base(p_symbol_func p_func, p_ir_vreg p_vreg) {
    assert(p_vreg->def_type == func_param_def);
    assert(p_vreg->p_func == p_func);
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_param_vmem_base p_base = list_entry(p_node, ir_param_vmem_base, node);
        assert(!p_base->p_param_base->is_vmem);
        if (p_vreg == p_base->p_vreg)
            return p_base;
    }
    assert(0);
}
void symbol_func_bb_add_tail(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    p_basic_block->p_func = p_func;
    if (p_basic_block->p_branch->kind == ir_ret_branch)
        p_func->p_ret_block = p_basic_block;
    if (list_head_alone(&p_func->block))
        p_func->p_entry_block = p_basic_block;
    list_add_prev(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
    p_func->if_updated_graph = true;
}
void symbol_func_bb_add_head(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    p_basic_block->p_func = p_func;
    if (p_basic_block->p_branch->kind == ir_ret_branch)
        p_func->p_ret_block = p_basic_block;
    if (list_head_alone(&p_func->block))
        p_func->p_entry_block = p_basic_block;
    list_add_next(&p_basic_block->node, &p_func->block);
    ++p_func->block_cnt;
    p_func->if_updated_graph = true;
}
void symbol_func_bb_del(p_symbol_func p_func, p_ir_basic_block p_basic_block) {
    assert(p_basic_block->p_func == p_func);
    assert(p_func->p_entry_block != p_basic_block);
    assert(p_func->p_ret_block != p_basic_block);
    ir_basic_block_drop(p_basic_block);
    p_func->if_updated_graph = true;
}
void symbol_func_param_vmem_base_drop(p_symbol_func p_func, p_ir_param_vmem_base p_base) {
    p_func->param_vmem_base_num--;
    ir_param_vmem_base_drop(p_base);
}
void symbol_func_param_reg_add(p_symbol_func p_func, p_ir_vreg p_vreg) {
    list_add_prev(&p_vreg->node, &p_func->param_reg_list);
    p_vreg->def_type = func_param_def;
    p_vreg->p_func = p_func;
    ++p_func->param_reg_cnt;
    if (p_vreg->p_type->ref_level) {
        symbol_func_param_vmem_base_add(p_func, ir_param_vmem_base_gen(p_vreg, p_func));
    }
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
    p_vmem->id = p_func->var_cnt++;
    assert(!p_vmem->p_func && !p_vmem->p_program);
    p_vmem->p_func = p_func;
    p_vmem->is_global = false;
    list_add_prev(&p_vmem->node, &p_func->param);
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

void symbol_func_add_variable(p_symbol_func p_func, p_symbol_var p_var) {
    p_var->id = p_func->var_cnt++;
    assert(!p_var->p_func && !p_var->p_program);
    p_var->p_func = p_func;
    p_var->is_global = false;
    assert(p_var->p_base);
    p_func->varray_num += p_var->p_base->num;
    list_add_prev(&p_var->node, &p_func->variable);
}

void symbol_func_drop(p_symbol_func p_func) {
    if (!list_head_alone(&p_func->block))
        assert(p_func->p_ret_block->p_branch->kind == ir_ret_branch);
    p_func->p_entry_block = NULL;
    p_func->p_ret_block = NULL;
    list_del(&p_func->node);
    while (!list_head_alone(&p_func->block)) {
        p_ir_basic_block p_del = list_entry(p_func->block.p_next, ir_basic_block, node);
        symbol_func_bb_del(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param_vmem_base)) {
        p_ir_param_vmem_base p_del = list_entry(p_func->param_vmem_base.p_next, ir_param_vmem_base, node);
        symbol_func_param_vmem_base_drop(p_func, p_del);
    }
    while (!list_head_alone(&p_func->param)) {
        p_symbol_var p_del = list_entry(p_func->param.p_next, symbol_var, node);
        assert(!p_del->is_global);
        assert(p_del->p_func == p_func);
        symbol_var_drop(p_del);
    }
    while (!list_head_alone(&p_func->variable)) {
        p_symbol_var p_del = list_entry(p_func->variable.p_next, symbol_var, node);
        assert(!p_del->is_global);
        assert(p_del->p_func == p_func);
        symbol_var_drop(p_del);
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
    ir_call_graph_node_drop(p_func);
    ir_side_effects_drop(p_func);
    func_loop_info_drop(p_func);
    assert(p_func->var_cnt == 0);
    assert(p_func->param_vmem_base_num == 0);
    assert(p_func->varray_num == 0);
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
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_func->param_vmem_base) {
        p_ir_param_vmem_base p_base = list_entry(p_node, ir_param_vmem_base, node);
        if (p_base->p_param_base->num == 0)
            symbol_func_param_vmem_base_drop(p_func, p_base);
    }
    list_for_each_safe(p_node, p_next, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (p_var->p_base->num == 0)
            symbol_var_drop(p_var);
    }
    list_for_each_safe(p_node, p_next, &p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (p_var->p_base->num == 0)
            symbol_var_drop(p_var);
    }
    symbol_func_set_varible_id(p_func);
}

void symbol_func_set_varible_id(p_symbol_func p_func) {
    size_t id = 0;
    p_list_head p_node, p_next;
    size_t param_vmem_num = 0;
    size_t varray_num = 0;
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_param_vmem_base p_param_base = list_entry(p_node, ir_param_vmem_base, node);
        ir_vmem_base_set_varray_id(p_param_base->p_param_base);
        p_param_base->id = param_vmem_num;
        varray_num += p_param_base->p_param_base->num;
        param_vmem_num++;
    }
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        assert(p_var->p_base);
        ir_vmem_base_set_varray_id(p_var->p_base);
        varray_num += p_var->p_base->num;
        p_var->id = id++;
    }
    list_for_each_safe(p_node, p_next, &p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        assert(p_var->p_base);
        ir_vmem_base_set_varray_id(p_var->p_base);
        varray_num += p_var->p_base->num;
        p_var->id = id++;
    }
    assert(id == p_func->var_cnt);
    assert(param_vmem_num == p_func->param_vmem_base_num);
    assert(varray_num == p_func->varray_num);
}
