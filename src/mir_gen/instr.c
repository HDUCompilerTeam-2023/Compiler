#include <mir_gen/instr.h>
#include <mir_gen.h>

p_mir_instr mir_binary_instr_gen(mir_instr_type mir_type, p_mir_operand p_src1, p_mir_operand p_src2, p_mir_symbol des)
{
    p_mir_instr p_instr = NULL;
    p_mir_binary_instr p_binary_instr = NULL;
    switch (mir_type) {
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
        p_binary_instr = malloc(sizeof(*p_binary_instr));
        *p_binary_instr = (mir_binary_instr){
            .p_src1 = p_src1,
            .p_src2 = p_src2,
            .p_des = des,
        };

        p_instr = malloc(sizeof(*p_instr));
        *p_instr = (mir_instr){
            .irkind = mir_type,
            .p_mir_binary = p_binary_instr,
            .node = list_head_init(&p_instr->node),
        };
        break;
        default:
            assert(0);
    }
    return p_instr;
}

p_mir_instr mir_unnary_instr_gen(mir_instr_type mir_type, p_mir_operand p_src, p_mir_symbol des)
{
    p_mir_instr p_instr = NULL;
    p_mir_unary_instr p_unary_instr = NULL;
    switch (mir_type) {
        case mir_minus_op:
        case mir_not_op:
        case mir_int2float_op:
        case mir_float2int_op:
        case mir_assign:

        p_unary_instr = malloc(sizeof(*p_unary_instr));
        *p_unary_instr = (mir_unary_instr){
            .p_src = p_src,
            .p_des = des,
        };

        p_instr = malloc(sizeof(*p_instr));
        *p_instr = (mir_instr){
            .irkind = mir_type,
            .p_mir_unary = p_unary_instr,
            .node = list_head_init(&p_instr->node),
        };
        break;
        default:
            assert(0);
    }
    return p_instr;
}

p_mir_instr mir_ret_instr_gen(p_mir_operand p_src)
{
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    p_mir_ret_instr p_ret_instr = malloc(sizeof(*p_ret_instr));
    
    *p_ret_instr = (mir_ret_instr){
        .p_ret = p_src,
    };
    *p_instr = (mir_instr){
        .irkind = mir_ret,
        .p_mir_ret = p_ret_instr,
        .node = list_head_init(&p_instr->node),
    };

    return p_instr;
}

p_mir_instr mir_br_instr_gen(p_mir_instr p_target)
{
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    p_mir_br_instr p_br_instr = malloc(sizeof(*p_br_instr));
    
    *p_br_instr = (mir_br_instr){
        .p_target = p_target,
    };
    *p_instr = (mir_instr){
        .irkind = mir_br,
        .p_mir_br = p_br_instr,
        .node = list_head_init(&p_instr->node),
    };

    return p_instr;
}

p_mir_instr mir_condbr_instr_gen(p_mir_operand p_cond, p_mir_instr p_target_false, p_mir_instr p_target_true)
{
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    p_mir_condbr_instr p_condbr_instr = malloc(sizeof(*p_condbr_instr));
    
    *p_condbr_instr = (mir_condbr_instr){
        .p_cond = p_cond,
        .p_target_false = p_target_false,
        .p_target_true = p_target_true,
    };
    *p_instr = (mir_instr){
        .irkind = mir_condbr,
        .p_mir_condbr = p_condbr_instr,
        .node = list_head_init(&p_instr->node),
    };

    return p_instr;
}

p_mir_instr mir_call_instr_gen(p_mir_symbol p_func_sym, p_mir_param_list p_param_list, p_mir_symbol p_des)
{
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    p_mir_call_instr p_call = malloc(sizeof(*p_call));
    *p_call = (mir_call_instr){
        .p_func = p_func_sym,
        .p_des = p_des,
    };
    *p_instr = (mir_instr){
        .p_mir_call = p_call,
        .node = list_head_init(&p_instr->node),
    };
    return p_instr;
}

p_mir_instr mir_array_instr_gen(p_mir_symbol p_array, p_mir_operand p_offset, p_mir_symbol p_des)
{
    p_mir_instr p_instr = malloc(sizeof(*p_instr));
    p_mir_array_instr p_array_instr = malloc(sizeof(*p_array_instr));
    *p_array_instr = (mir_array_instr){
        .p_array = p_array,
        .p_offset = p_offset,
        .p_des = p_des,
    };
    *p_instr = (mir_instr){
        .p_mir_array = p_array_instr,
        .node = list_head_init(&p_instr->node),
    };
    return p_instr;
}

void mir_instr_drop(p_mir_instr p_instr)
{
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
            mir_operand_drop(p_instr->p_mir_binary->p_src1);
            mir_operand_drop(p_instr->p_mir_binary->p_src2);
            mir_symbol_drop(p_instr->p_mir_binary->p_des);
            free(p_instr->p_mir_binary);
            break;
        case mir_minus_op:
        case mir_not_op:
        case mir_int2float_op:
        case mir_float2int_op:
        case mir_assign:
            mir_operand_drop(p_instr->p_mir_unary->p_src);
            mir_symbol_drop(p_instr->p_mir_unary->p_des);
            free(p_instr->p_mir_unary);
            break;
        case mir_call:
            mir_param_list_drop(p_instr->p_mir_call->p_param_list);
            mir_symbol_drop(p_instr->p_mir_unary->p_des);
            free(p_instr->p_mir_call);
            break;
        case mir_array:
            mir_symbol_drop(p_instr->p_mir_array->p_array);// maybe not need ?
            mir_operand_drop(p_instr->p_mir_array->p_offset);
            mir_symbol_drop(p_instr->p_mir_array->p_des);
            free(p_instr->p_mir_array);
            break;
        case mir_ret:
            mir_operand_drop(p_instr->p_mir_ret->p_ret);
            free(p_instr->p_mir_ret);
            break;
        case mir_br:
            free(p_instr->p_mir_br);
            break;
       case mir_condbr:
            mir_operand_drop(p_instr->p_mir_condbr->p_cond);
            free(p_instr->p_mir_condbr);
            break;
    }
}
