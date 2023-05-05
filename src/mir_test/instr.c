#include <mir.h>
#include <mir/basic_block.h>
#include <mir/func.h>
#include <mir/instr.h>
#include <mir/operand.h>
#include <mir/param.h>
#include <mir/program.h>
#include <mir/temp_sym.h>

#include <mir_port.h>

#include <mir_test.h>

#include <util.h>

#include <symbol/store.h>
#include <symbol/sym.h>
#include <symbol/type.h>

memory_type mir_instr_test(p_list_head p_node, memory_stack **global_stack, memory_stack **top_stack) {
    memory_type re = {};
    p_list_head p_head = p_node;
    p_node = p_node->p_next;
    p_mir_instr p_instr = NULL;
    while (p_node != p_head) {
        p_instr = list_entry(p_node, mir_instr, node);
        assert(p_instr);
        switch (p_instr->irkind) {
        case mir_add_op:
        case mir_sub_op:
        case mir_mul_op:
        case mir_div_op:
        case mir_mod_op:
        case mir_and_op:
        case mir_or_op:
        case mir_eq_op:
        case mir_neq_op:
        case mir_l_op:
        case mir_leq_op:
        case mir_g_op:
        case mir_geq_op:
            mir_binary_instr_test(p_instr->irkind, &p_instr->mir_binary, *global_stack, *top_stack);
            break;

        case mir_minus_op:
        case mir_not_op:
        case mir_int2float_op:
        case mir_float2int_op:
        case mir_val_assign:
            mir_unary_instr_test(p_instr->irkind, &p_instr->mir_unary, *global_stack, *top_stack);
            break;
        case mir_ret:
            re = mir_ret_instr_test(&p_instr->mir_ret, *global_stack, *top_stack);
            break;
        case mir_call:
            mir_call_instr_test(&p_instr->mir_call, &(*global_stack), &(*top_stack));
            break;
        case mir_array:
            mir_array_instr_test(&p_instr->mir_array, *global_stack, *top_stack);
            break;
        case mir_array_assign:
            mir_array_assign_instr_test(&p_instr->mir_array_assign, *global_stack, *top_stack);
            break;
        case mir_br:
            p_head = &mir_instr_br_get_target(p_instr)->instr_list;
            p_node = p_head->p_next;
            continue;

        case mir_condbr:
            if (mir_operand_data_get((&p_instr->mir_condbr)->p_cond, *global_stack, *top_stack).i)
                p_head = &mir_instr_condbr_get_target_true(p_instr)->instr_list;
            else
                p_head = &mir_instr_condbr_get_target_false(p_instr)->instr_list;
            p_node = p_head->p_next;
            continue;
        }
        (p_node) = (p_node)->p_next;
    }
    return re;
}

void mir_binary_instr_test(mir_instr_type instr_type, p_mir_binary_instr p_instr, memory_stack *global_stack, memory_stack *top_stack) {
    memory_type src1 = mir_operand_data_get(p_instr->p_src1, global_stack, top_stack);
    memory_type src2 = mir_operand_data_get(p_instr->p_src2, global_stack, top_stack);
    if (src2.type == I32 && src1.type == I32) {
        int res;
        switch (instr_type) {
        case mir_add_op:
            res = src1.i + src2.i;
            break;
        case mir_sub_op:
            res = src1.i - src2.i;
            break;
        case mir_mul_op:
            res = src1.i * src2.i;
            break;
        case mir_div_op:
            res = src1.i / src2.i;
            break;
        case mir_mod_op:
            res = src1.i % src2.i;
            break;
        case mir_and_op:
            res = src1.i && src2.i;
            break;
        case mir_or_op:
            res = src1.i || src2.i;
            break;
        case mir_eq_op:
            res = src1.i == src2.i;
            break;
        case mir_neq_op:
            res = src1.i != src2.i;
            break;
        case mir_l_op:
            res = src1.i < src2.i;
            break;
        case mir_leq_op:
            res = src1.i <= src2.i;
            break;
        case mir_g_op:
            res = src1.i > src2.i;
            break;
        case mir_geq_op:
            res = src1.i >= src2.i;
            break;
        default:
            assert(0);
        }
        if (p_instr->p_des->kind == temp_var) {
            //printf("t%zu = %d\n", p_instr->p_des->p_temp_sym->id, res);
            top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id].i = res;
            if (p_instr->p_des->p_temp_sym->is_pointer) {
                //printf("%d\n", res);
                int arry_size = p_instr->p_src1->p_sym->p_type->size - src2.i;
                if (p_instr->p_src1->p_sym->is_global)
                    arry_size = -arry_size;
                top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id].type = arry_size;
            }
            else
                top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id].type = I32;
        }
        else {
            if (p_instr->p_des->p_sym->is_global)
                if (global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = res;
                else
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = res;
            else if (top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = res;
            else
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = res;
        }
    }
    else {
        if (src1.type == I32) src1.f = (float) src1.i;
        if (src2.type == I32) src2.f = (float) src2.i;
        float res;
        switch (instr_type) {
        case mir_add_op:
            res = src1.f + src2.f;
            break;
        case mir_sub_op:
            res = src1.f - src2.f;
            break;
        case mir_mul_op:
            res = src1.f * src2.f;
            break;
        case mir_div_op:
            res = src1.f / src2.f;
            break;
        case mir_mod_op:
            break;
        case mir_and_op:
            res = src1.f && src2.f;
            break;
        case mir_or_op:
            res = src1.f || src2.f;
            break;
        case mir_eq_op:
            res = src1.f == src2.f;
            break;
        case mir_neq_op:
            res = src1.f != src2.f;
            break;
        case mir_l_op:
            res = src1.f < src2.f;
            break;
        case mir_leq_op:
            res = src1.f <= src2.f;
            break;
        case mir_g_op:
            res = src1.f > src2.f;
            break;
        case mir_geq_op:
            res = src1.f >= src2.f;
            break;
        default:
            assert(0);
        }
        if (p_instr->p_des->kind == temp_var) {
            top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id].f = res;
            top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id].type = F32;
        }
        else {
            if (p_instr->p_des->p_sym->is_global)
                if (global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = res;
                else
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = res;
            else if (top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = res;
            else
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = res;
        }
    }
}

void mir_unary_instr_test(mir_instr_type instr_type, p_mir_unary_instr p_instr, memory_stack *global_stack, memory_stack *top_stack) {
    memory_type scr = mir_operand_data_get(p_instr->p_src, global_stack, top_stack);
    if (scr.type == I32) {
        switch (instr_type) {
        case mir_minus_op:
            scr.i = -scr.i;
            break;
        case mir_not_op:
            scr.i = !scr.i;
            break;
        case mir_int2float_op:
            scr.f = (float) scr.i;
            scr.type = F32;
            break;
        case mir_float2int_op:
            break;
        case mir_val_assign:;
            break;
        default:
            assert(0);
        }
        if (p_instr->p_des->kind == temp_var) {
            top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id] = scr;
        }
        else {
            if (p_instr->p_des->p_sym->is_global) {

                if (global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                    if (scr.type == I32)
                        global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = scr.i;
                    else
                        global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = scr.f;
                else if (scr.type == I32)
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = scr.i;
                else
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = scr.f;
            }
            else {
                if (top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].type == I32) {
                    if (scr.type == I32)
                        top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = scr.i;
                    else
                        top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = scr.f;
                }
                else {
                    if (scr.type == I32)
                        top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = scr.i;
                    else
                        top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = scr.f;
                }
            }
        }
    }
    else {
        switch (instr_type) {
        case mir_minus_op:
            scr.f = -scr.f;
            break;
        case mir_not_op:
            scr.f = !scr.f;
            break;
        case mir_int2float_op:
            break;
        case mir_float2int_op:
            scr.i = (int) scr.f;
            scr.type = I32;
            break;
        case mir_val_assign:
            break;
        default:
            assert(0);
        }
        if (p_instr->p_des->kind == temp_var) {
            top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id] = scr;
        }
        else {
            if (p_instr->p_des->p_sym->is_global)
                if (global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                    if (scr.type == I32)
                        global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = scr.i;
                    else
                        global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].i = scr.f;
                else if (scr.type == I32)
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = scr.i;
                else
                    global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]].f = scr.f;
            else if (top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].type == I32)
                if (scr.type == I32)
                    top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = scr.i;
                else
                    top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].i = scr.f;
            else if (scr.type == I32)
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = scr.i;
            else
                top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]].f = scr.f;
        }
    }
}

void mir_array_assign_instr_test(p_mir_array_assign_instr p_instr, memory_stack *global_stack, memory_stack *top_stack) {
    assert(p_instr->p_array->kind == declared_var);
    memory_type scr = mir_operand_data_get(p_instr->p_src, global_stack, top_stack);
    memory_type offset = mir_operand_data_get(p_instr->p_offset, global_stack, top_stack);
    if (p_instr->p_array->p_sym->is_global) {
        int des = global_stack->pos[p_instr->p_array->p_sym->id] + offset.i;
        if (scr.type == I32) {
            if (p_instr->p_array->p_sym->p_type->basic == type_int)
                global_stack->mem[des].i = scr.i;
            else
                global_stack->mem[des].f = scr.i;
        }
        else {
            if (p_instr->p_array->p_sym->p_type->basic == type_int)
                global_stack->mem[des].i = scr.f;
            else
                global_stack->mem[des].f = scr.f;
        }
    }
    else {
        int des = top_stack->pos[p_instr->p_array->p_sym->id] + offset.i;
        if (scr.type == I32) {
            if (top_stack->mem[des].type == I32)
                top_stack->mem[des].i = scr.i;
            else
                top_stack->mem[des].f = scr.i;
        }
        else {
            //printf("%d\n", top_stack->mem->type);
            if (top_stack->mem[des].type == I32)
                top_stack->mem[des].i = scr.f;
            else
                top_stack->mem[des].f = scr.f;
        }
        //printf("%d = %lld\n", des, top_stack->mem[des].i);
    }
}

void mir_array_instr_test(p_mir_array_instr p_instr, memory_stack *global_stack, memory_stack *top_stack) {
    assert(p_instr->p_array->kind == declared_var);
    memory_type *des = NULL;
    switch (p_instr->p_des->kind) {
    case temp_var:
        des = &top_stack->mem[top_stack->mem_cnt + p_instr->p_des->p_temp_sym->id];
        if (p_instr->p_des->p_temp_sym->b_type == type_int) des->type = I32;
        else
            des->type = F32;
        break;
    case declared_var:
        if (p_instr->p_des->p_sym->is_global)
            des = &global_stack->mem[global_stack->pos[p_instr->p_des->p_sym->id]];
        else
            des = &top_stack->mem[top_stack->pos[p_instr->p_des->p_sym->id]];
        break;
    default:
        break;
    }
    if (des == NULL) return;
    memory_type scr;
    memory_type offset = mir_operand_data_get(p_instr->p_offset, global_stack, top_stack);
    if (p_instr->p_array->p_sym->is_global)
        scr = global_stack->mem[(int) (global_stack->pos[p_instr->p_array->p_sym->id] + offset.i)];
    else
        scr = top_stack->mem[(int) (top_stack->pos[p_instr->p_array->p_sym->id] + offset.i)];
    if (scr.type == I32) {
        if (des->type == I32)
            des->i = scr.i;
        else
            des->f = scr.i;
    }
    else {
        if (des->type == I32)
            des->i = scr.f;
        else
            des->f = scr.f;
    }
}

memory_type mir_ret_instr_test(p_mir_ret_instr p_instr, memory_stack *global_stack, memory_stack *top_stack) {
    memory_type re = mir_operand_data_get(p_instr->p_ret, global_stack, top_stack);
    if (re.type == I32)
        printf("%d\n", re.i);
    else
        printf("%f\n", re.f);
    return re;
}

void mir_call_instr_test(p_mir_call_instr p_instr, memory_stack **global_stack, memory_stack **top_stack) {

    if (list_head_alone(&p_instr->p_func->entry_block)) return;
    stack_push(&(*top_stack));
    mir_param_list_test(p_instr->p_param_list, *global_stack, *top_stack);
    memory_type ans = mir_func_test(p_instr->p_func, &(*global_stack), &(*top_stack));
    (*top_stack)->mem[p_instr->p_des->p_temp_sym->id + (*top_stack)->mem_cnt] = ans;
}