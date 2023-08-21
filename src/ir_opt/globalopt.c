#include <ir/basic_block.h>
#include <ir/bb_param.h>
#include <ir/operand.h>
#include <ir/param.h>
#include <ir/varray.h>
#include <ir/vreg.h>
#include <ir_gen/basic_block.h>
#include <ir_gen/bb_param.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir_gen/varray.h>
#include <ir_opt/deadcode_elimate.h>
#include <program/gen.h>
#include <program/use.h>
#include <symbol/type.h>
#include <symbol_gen/func.h>
#include <symbol_gen/var.h>

static inline void _global_opt_var_list(p_list_head p_list) {
    p_list_head p_node;
    list_for_each(p_node, p_list) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        p_mem_info p_info = p_var->p_visited;

        if (list_head_alone(&p_info->load_list)) {
            printf("%s hasent be load\n", p_var->name);
            ir_vmem_base_clear_all(p_var->p_base);
        }
        else if (list_head_alone(&p_info->store_list)) {
            printf("%s hasent be store\n", p_var->name);
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &p_info->load_list) {
                p_mem_visit_instr_node p_instr_node = list_entry(p_node, mem_visit_instr_node, node);
                p_ir_instr p_instr = p_instr_node->p_instr;
                if (p_instr->irkind == ir_call)
                    continue;
                assert(p_instr->irkind == ir_load);
                p_ir_operand p_init = NULL;
                if (p_info->p_var->p_init) {
                    assert(p_info->p_var->is_global);
                    size_t off = 0;
                    if (p_info->p_var->p_type->ref_level == 0 && !list_head_alone(&p_info->p_var->p_type->array)) {
                        if (p_instr->ir_load.p_addr->kind == reg)
                            continue;
                        off = p_instr->ir_load.p_addr->offset >> 2;
                    }
                    if (p_info->p_var->p_type->basic == type_i32)
                        p_init = ir_operand_int_gen(p_info->p_var->p_init->memory[off].i);
                    else {
                        assert(p_info->p_var->p_type->basic == type_f32);
                        p_init = ir_operand_float_gen(p_info->p_var->p_init->memory[off].f);
                    }
                }
                else {
                    if (p_info->p_var->p_type->basic == type_i32)
                        p_init = ir_operand_int_gen(0);
                    else {
                        assert(p_info->p_var->p_type->basic == type_f32);
                        p_init = ir_operand_float_gen(0);
                    }
                }
                ir_instr_reset_unary(p_instr, ir_val_assign, p_init, p_instr->ir_load.p_des);
            }
        }
    }
}

static inline void revise_mem(p_ir_operand p_operand) {
    p_ir_vreg p_des = NULL;
    switch (p_operand->used_type) {
    case instr_ptr:
        switch (p_operand->p_instr->irkind) {
        case ir_store:
            p_operand->p_instr->ir_store.is_stack_ptr = true;
            break;
        case ir_load:
            p_operand->p_instr->ir_load.is_stack_ptr = true;
            break;
        case ir_unary:
            p_des = p_operand->p_instr->ir_unary.p_des;
            break;
        case ir_binary:
            p_des = p_operand->p_instr->ir_binary.p_des;
            break;
        case ir_gep:
            p_des = p_operand->p_instr->ir_gep.p_des;
            break;
        case ir_call:
            assert(0);
            break;
        }
        break;
    case bb_param_ptr:
    case cond_ptr:
    case ret_ptr:
        assert(0);
        break;

    }
    if(!p_des)
        return;
    p_list_head p_node;
    list_for_each(p_node, &p_des->use_list) {
        p_ir_operand p_use = list_entry(p_node, ir_operand, use_node);
        revise_mem(p_use);
    }
}
static inline void _global_variable_localize(p_symbol_var p_var) {
    assert(p_var->is_global);
    if (p_var->p_type->ref_level == 0 && !list_head_alone(&p_var->p_type->array))
        return;
    p_symbol_func p_func = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_var->ref_list) {
        p_ir_operand p_ref = list_entry(p_node, ir_operand, ref_node);
        assert(p_ref->kind == imme);
        assert(p_ref->p_type->ref_level > 0);
        assert(p_ref->p_vmem == p_var);

        p_symbol_func p_use_func = NULL;
        switch (p_ref->used_type) {
        case instr_ptr:
            p_use_func = p_ref->p_instr->p_basic_block->p_func;
            break;
        case bb_param_ptr:
            p_use_func = p_ref->p_bb_param->p_target->p_source_block->p_func;
            break;
        case cond_ptr:
        case ret_ptr:
            p_use_func = p_ref->p_basic_block->p_func;
            break;
        }
        assert(p_use_func);
        if (!p_func) {
            p_func = p_use_func;
        }
        if (p_func != p_use_func)
            break;
    }
    if (!p_func)
        return;
    if (p_node == &p_var->ref_list) {
        if (strcmp(p_func->name, "main"))
            return;
        ir_vmem_base_clear_all(p_var->p_base);
        symbol_var_del(p_var);
        symbol_func_add_variable(p_func, p_var);
        assert(p_var->p_type->ref_level > 0 || list_head_alone(&p_var->p_type->array));
        p_ir_operand p_init = NULL;
        if (p_var->p_init) {
            if (p_var->p_type->basic == type_i32)
                p_init = ir_operand_int_gen(p_var->p_init->memory->i);
            else {
                assert(p_var->p_type->basic == type_f32);
                p_init = ir_operand_float_gen(p_var->p_init->memory->f);
            }
            symbol_init_drop(p_var->p_init);
            p_var->p_init = NULL;
        }
        else {
            if (p_var->p_type->basic == type_i32)
                p_init = ir_operand_int_gen(0);
            else {
                assert(p_var->p_type->basic == type_f32);
                p_init = ir_operand_float_gen(0);
            }
        }
        p_ir_instr p_instr = ir_store_instr_gen(ir_operand_addr_gen(p_var, NULL, 0), p_init, true);
        ir_basic_block_addinstr_head(p_func->p_entry_block, p_instr);
        p_list_head p_node;
        list_for_each(p_node, &p_var->ref_list) {
            p_ir_operand p_operand = list_entry(p_node, ir_operand, ref_node);
            revise_mem(p_operand);
        }
        assert(!p_var->is_global);
        assert(p_var->p_func == p_func);
    }
}

void ir_opt_globalopt(p_program p_ir) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_ir->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        _global_variable_localize(p_var);
    }

    ir_side_effects(p_ir);
    _global_opt_var_list(&p_ir->variable);

    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        _global_opt_var_list(&p_func->variable);
    }
}
