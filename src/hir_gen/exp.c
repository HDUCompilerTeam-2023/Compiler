#include <hir_gen/exp.h>

#include <hir_gen.h>

#include <symbol_gen.h>

static inline p_hir_exp exp_ptr_to_val(p_hir_exp p_exp) {
    if (p_exp->p_type->ref_level == 0) {
        assert(list_head_alone(&p_exp->p_type->array));
        return p_exp;
    }
    if (p_exp->p_type->ref_level > 1) {
        return hir_exp_load_gen(p_exp);
    }
    // assert(p_exp->p_type->ref_level == 1);
    if (list_head_alone(&p_exp->p_type->array)) {
        return hir_exp_load_gen(p_exp);
    }
    return hir_exp_gep_gen(p_exp, hir_exp_int_gen(0), true);
}

static inline p_hir_exp exp_ptr_to_val_check_basic(p_hir_exp p_exp) {
    p_exp = exp_ptr_to_val(p_exp);
    assert(list_head_alone(&p_exp->p_type->array) && p_exp->p_type->ref_level == 0);
    assert(p_exp->p_type->basic != type_void);
    return p_exp;
}

static inline p_hir_exp exp_val_const(p_hir_exp p_exp) {
    if (p_exp->p_type->ref_level != 0) return p_exp;
    assert(list_head_alone(&p_exp->p_type->array));

    p_hir_exp p_back = p_exp;
    size_t offset = 0;
    size_t length = 1;
    p_exp = p_exp->p_src_1;
    while (p_exp->kind == hir_exp_gep) {
        if (p_exp->p_offset->kind != hir_exp_num)
            break;
        if (p_exp->p_offset->p_type->basic != type_int)
            break;
        length *= symbol_type_get_size(p_exp->p_type);
        offset += length * p_exp->p_offset->intconst;
        p_exp = p_exp->p_addr;
    }
    if (p_exp->kind == hir_exp_ptr && p_exp->p_var->is_const) {
        assert(offset < symbol_type_get_size(p_exp->p_var->p_type));
        p_hir_exp p_val;
        if (p_exp->p_type->basic == type_int) {
            p_val = hir_exp_int_gen(p_exp->p_var->p_init->memory[offset].i);
        }
        else {
            assert(p_exp->p_var->p_init);
            p_val = hir_exp_float_gen(p_exp->p_var->p_init->memory[offset].f);
        }
        hir_exp_drop(p_back);

        return p_val;
    }
    return p_back;
}

void hir_exp_ptr_check_lval(p_hir_exp p_exp) {
    assert(list_head_alone(&p_exp->p_type->array));
    assert(p_exp->p_type->ref_level == 1);
    assert(p_exp->p_type->basic != type_void);
}
p_hir_exp hir_exp_ptr_to_val_check_basic(p_hir_exp p_exp) {
    return exp_ptr_to_val_check_basic(p_exp);
}
p_hir_exp hir_exp_ptr_to_val(p_hir_exp p_exp) {
    return exp_ptr_to_val(p_exp);
}

p_hir_exp hir_exp_use_gen(p_hir_exp p_used_exp) {
    assert(p_used_exp);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_use,
        .p_exp = p_used_exp,
        .p_type = symbol_type_copy(p_used_exp->p_type),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_binary_gen(hir_exp_binary_op b_op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    p_src_1 = exp_ptr_to_val_check_basic(p_src_1);
    p_src_2 = exp_ptr_to_val_check_basic(p_src_2);
    if (b_op == hir_exp_op_mod) assert(p_src_1->p_type->basic == type_int && p_src_2->p_type->basic == type_int);

    basic_type type = p_src_1->p_type->basic;
    if (p_src_2->p_type->basic == type_float) {
        type = type_float;
    }

    if (p_src_1->kind == hir_exp_num && p_src_2->kind == hir_exp_num) {
        switch (b_op) {
        case hir_exp_op_add:
            if (p_src_1->p_type->basic == type_int) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst + p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->p_type->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst + p_src_2->floatconst;
                }
            }
            else if (p_src_1->p_type->basic == type_float) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst + p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst + p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_sub:
            if (p_src_1->p_type->basic == type_int) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst - p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->p_type->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst - p_src_2->floatconst;
                }
            }
            else if (p_src_1->p_type->basic == type_float) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst - p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst - p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_mul:
            if (p_src_1->p_type->basic == type_int) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst * p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->p_type->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst * p_src_2->floatconst;
                }
            }
            else if (p_src_1->p_type->basic == type_float) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst * p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst * p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_div:
            if (p_src_1->p_type->basic == type_int) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst / p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->p_type->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst / p_src_2->floatconst;
                }
            }
            else if (p_src_1->p_type->basic == type_float) {
                if (p_src_2->p_type->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst / p_src_2->intconst;
                }
                else if (p_src_2->p_type->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst / p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_mod:
            p_src_1->intconst = p_src_1->intconst % p_src_2->intconst;
            break;
        default:
            assert(1);
        }
        hir_exp_drop(p_src_2);
        return p_src_1;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_binary,
        .b_op = b_op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .p_type = symbol_type_var_gen(type),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_relational_gen(hir_exp_binary_op b_op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    p_src_1 = exp_ptr_to_val_check_basic(p_src_1);
    p_src_2 = exp_ptr_to_val_check_basic(p_src_2);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_binary,
        .b_op = b_op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .p_type = symbol_type_var_gen(type_int),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_logic_gen(hir_exp_logic_op l_op, p_hir_exp p_bool_1, p_hir_exp p_bool_2) {
    assert(p_bool_1 && p_bool_2);
    p_bool_1 = exp_ptr_to_val_check_basic(p_bool_1);
    p_bool_2 = exp_ptr_to_val_check_basic(p_bool_2);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_logic,
        .l_op = l_op,
        .p_bool_1 = p_bool_1,
        .p_bool_2 = p_bool_2,
        .p_type = symbol_type_var_gen(type_int),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_ulogic_gen(hir_exp_ulogic_op ul_op, p_hir_exp p_bool) {
    assert(p_bool);
    p_bool = exp_ptr_to_val_check_basic(p_bool);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_ulogic,
        .ul_op = ul_op,
        .p_bool = p_bool,
        .p_type = symbol_type_var_gen(type_int),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_unary_gen(hir_exp_unary_op u_op, p_hir_exp p_src) {
    assert(p_src);
    p_src = exp_ptr_to_val_check_basic(p_src);

    basic_type type = p_src->p_type->basic;
    if (u_op == hir_exp_op_bool_not) {
        type = type_int;
    }

    if (p_src->kind == hir_exp_num) {
        switch (u_op) {
        case hir_exp_op_minus:
            if (p_src->p_type->basic == type_int) p_src->intconst = -p_src->intconst;
            else if (p_src->p_type->basic == type_float)
                p_src->floatconst = -p_src->floatconst;
            break;
        default:
            assert(1);
        }
        return p_src;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_unary,
        .u_op = u_op,
        .p_src = p_src,
        .p_type = symbol_type_var_gen(type),
        .p_des = NULL,
    };
    return p_exp;
}

static inline bool param_arr_check(p_symbol_type p_type_1, p_symbol_type p_type_2) {
    if (p_type_1->ref_level != p_type_2->ref_level) return false;
    if (p_type_1->basic != p_type_2->basic) return false;
    p_list_head p_node_1, p_node_2 = p_type_2->array.p_next;
    list_for_each(p_node_1, &p_type_1->array) {
        if (p_node_2 == &p_type_2->array) return false;
        p_symbol_type_array p_array_1, p_array_2;
        p_array_1 = list_entry(p_node_1, symbol_type_array, node);
        p_array_2 = list_entry(p_node_2, symbol_type_array, node);
        p_node_2 = p_node_2->p_next;
        if (symbol_type_array_get_size(p_array_1) == 0 || symbol_type_array_get_size(p_array_2) == 0) continue;
        if (symbol_type_array_get_size(p_array_1) != symbol_type_array_get_size(p_array_2)) return false;
    }
    return (p_node_2 == &p_type_2->array);
}

p_hir_exp hir_exp_call_gen(p_symbol_func p_func, p_hir_param_list p_param_list) {
    assert(p_func);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_call,
        .p_func = p_func,
        .p_param_list = p_param_list,
        .p_type = symbol_type_var_gen(p_func->ret_type),
        .p_des = NULL,
    };

    p_list_head p_node_Fparam = p_func->param.p_next;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param) {
        if (p_node_Fparam == &p_func->param) {
            assert(p_func->is_va);
            break;
        }
        p_symbol_type p_param_type = list_entry(p_node_Fparam, symbol_var, node)->p_type;

        p_hir_exp p_param = list_entry(p_node, hir_param, node)->p_exp;
        assert(param_arr_check(p_param_type, p_param->p_type));
        p_node_Fparam = p_node_Fparam->p_next;
    }
    assert(p_node_Fparam == &p_func->param);
    return p_exp;
}

p_hir_exp hir_exp_ptr_gen(p_symbol_var p_var) {
    assert(p_var);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_ptr,
        .p_var = p_var,
        .p_type = symbol_type_copy(p_var->p_type),
        .p_des = NULL,
    };
    symbol_type_push_ptr(p_exp->p_type);
    return p_exp;
}
p_hir_exp hir_exp_gep_gen(p_hir_exp p_val, p_hir_exp p_offset, bool is_element) {
    assert(p_val->p_type->ref_level == 1);
    assert(!is_element || !list_head_alone(&p_val->p_type->array));
    p_offset = exp_ptr_to_val_check_basic(p_offset);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_gep,
        .p_addr = p_val,
        .p_offset = p_offset,
        .is_element =  is_element,
        .p_type = symbol_type_copy(p_val->p_type),
        .p_des = NULL,
    };
    if (is_element) {
        symbol_type_pop_ptr(p_exp->p_type);
        symbol_type_array_drop(symbol_type_pop_array(p_exp->p_type));
        symbol_type_push_ptr(p_exp->p_type);
    }

    return p_exp;
}
p_hir_exp hir_exp_load_gen(p_hir_exp p_ptr) {
    assert(p_ptr->p_type->ref_level > 0);
    if (p_ptr->p_type->ref_level == 1) {
        assert(list_head_alone(&p_ptr->p_type->array));
    }
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_load,
        .p_ptr = p_ptr,
        .p_type = symbol_type_copy(p_ptr->p_type),
        .p_des = NULL,
    };
    symbol_type_pop_ptr(p_exp->p_type);

    return exp_val_const(p_exp);
}

p_hir_exp hir_exp_int_gen(INTCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .intconst = num,
        .p_type = symbol_type_var_gen(type_int),
        .p_des = NULL,
    };
    return p_exp;
}
p_hir_exp hir_exp_float_gen(FLOATCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .floatconst = num,
        .p_type = symbol_type_var_gen(type_float),
        .p_des = NULL,
    };
    return p_exp;
}

p_hir_exp hir_exp_str_gen(p_symbol_str p_str) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .p_str = p_str,
        .p_type = symbol_type_var_gen(type_str),
        .p_des = NULL,
    };
    return p_exp;
}

void hir_exp_drop(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_binary:
        switch (p_exp->b_op) {
        case hir_exp_op_add:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_sub:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_mul:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_div:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_mod:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_eq:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_neq:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_l:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_leq:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_g:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_geq:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        }
        break;
    case hir_exp_unary:
        switch (p_exp->u_op) {
        case hir_exp_op_minus:
            hir_exp_drop(p_exp->p_src);
            break;
        }
        break;
    case hir_exp_logic:
        switch (p_exp->l_op) {
        case hir_exp_op_bool_and:
            hir_exp_drop(p_exp->p_bool_1);
            hir_exp_drop(p_exp->p_bool_2);
            break;
        case hir_exp_op_bool_or:
            hir_exp_drop(p_exp->p_bool_1);
            hir_exp_drop(p_exp->p_bool_2);
            break;
        }
        break;
    case hir_exp_ulogic:
        switch (p_exp->ul_op) {
        case hir_exp_op_bool_not:
            hir_exp_drop(p_exp->p_bool);
            break;
        }
        break;
    case hir_exp_call:
        hir_param_list_drop(p_exp->p_param_list);
        break;
    case hir_exp_ptr:
        break;
    case hir_exp_gep:
        hir_exp_drop(p_exp->p_addr);
        hir_exp_drop(p_exp->p_offset);
        break;
    case hir_exp_load:
        hir_exp_drop(p_exp->p_ptr);
        break;
    case hir_exp_num:
        break;
    case hir_exp_use:
        break;
    }
    symbol_type_drop(p_exp->p_type);
    free(p_exp);
}
