#include <hir_gen/exp.h>

#include <hir_gen.h>

#include <symbol_gen.h>

static inline basic_type exp_basic(p_hir_exp p_exp) {
    if (p_exp->kind == hir_exp_call) {
        assert(p_exp->p_func->p_sym->p_type->kind >= type_func);
        return p_exp->p_func->p_sym->p_type->basic;
    }
    if (p_exp->kind == hir_exp_val) {
        assert(p_exp->p_type->kind == type_var);
        return p_exp->p_type->basic;
    }
    if (p_exp->kind == hir_exp_str) {
        return type_str;
    }
    return p_exp->basic;
}

static inline p_hir_exp exp_val_const(p_hir_exp p_exp) {
    if (!p_exp->p_sym->is_const)
        return p_exp;
    if (p_exp->p_type->kind == type_arrary)
        return p_exp;

    size_t offset = 0;

    if (!p_exp->p_offset) {
        goto to_const;
    }

    if (p_exp->p_offset->kind != hir_exp_num)
        return p_exp;

    assert(p_exp->p_offset->basic == type_int);
    offset = p_exp->p_offset->intconst;

to_const:
    assert(offset < p_exp->p_sym->p_type->size);
    p_hir_exp p_val;
    if (p_exp->p_type->basic == type_int) {
        p_val = hir_exp_int_gen(p_exp->p_sym->p_init->memory[offset].i);
    }
    else {
        p_val = hir_exp_float_gen(p_exp->p_sym->p_init->memory[offset].f);
    }
    hir_exp_drop(p_exp);
    return p_val;
}

basic_type hir_exp_get_basic(p_hir_exp p_exp) {
    return exp_basic(p_exp);
}
p_hir_exp hir_exp_assign_gen(p_hir_exp lval, p_hir_exp rval) {
    assert(lval && rval);
    assert(lval->kind == hir_exp_val);
    assert(exp_basic(lval) != type_void);
    assert(exp_basic(rval) != type_void);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_assign,
        .p_src_1 = lval,
        .p_src_2 = rval,
        .basic = exp_basic(lval),
    };
    return p_exp;
}
p_hir_exp hir_exp_exec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    assert(exp_basic(p_src_1) != type_void);
    assert(exp_basic(p_src_2) != type_void);
    if (op == hir_exp_op_mod) assert(exp_basic(p_src_1) == type_int && exp_basic(p_src_1) == type_int);

    basic_type type = exp_basic(p_src_1);
    if (exp_basic(p_src_2) == type_float) {
        type = type_float;
    }

    if (p_src_1->kind == hir_exp_num && p_src_2->kind == hir_exp_num) {
        switch (op) {
        case hir_exp_op_add:
            if (p_src_1->basic == type_int) {
                if (p_src_2->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst + p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst + p_src_2->floatconst;
                }
            }
            else if (p_src_1->basic == type_float) {
                if (p_src_2->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst + p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst + p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_sub:
            if (p_src_1->basic == type_int) {
                if (p_src_2->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst - p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst - p_src_2->floatconst;
                }
            }
            else if (p_src_1->basic == type_float) {
                if (p_src_2->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst - p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst - p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_mul:
            if (p_src_1->basic == type_int) {
                if (p_src_2->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst * p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst * p_src_2->floatconst;
                }
            }
            else if (p_src_1->basic == type_float) {
                if (p_src_2->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst * p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->floatconst = p_src_1->floatconst * p_src_2->floatconst;
                }
            }
            break;
        case hir_exp_op_div:
            if (p_src_1->basic == type_int) {
                if (p_src_2->basic == type_int) {
                    p_src_1->intconst = p_src_1->intconst / p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
                    p_src_1->basic = type_float;
                    p_src_1->floatconst = p_src_1->intconst / p_src_2->floatconst;
                }
            }
            else if (p_src_1->basic == type_float) {
                if (p_src_2->basic == type_int) {
                    p_src_1->floatconst = p_src_1->floatconst / p_src_2->intconst;
                }
                else if (p_src_2->basic == type_float) {
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
        free(p_src_2);
        return p_src_1;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type,
    };
    return p_exp;
}
p_hir_exp hir_exp_lexec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    assert(exp_basic(p_src_1) != type_void);
    assert(exp_basic(p_src_2) != type_void);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type_int,
    };
    return p_exp;
}
p_hir_exp hir_exp_uexec_gen(hir_exp_op op, p_hir_exp p_src_1) {
    assert(p_src_1);
    assert(exp_basic(p_src_1) != type_void);

    basic_type type = exp_basic(p_src_1);
    if (op == hir_exp_op_bool_not) {
        type = type_int;
    }

    if (p_src_1->kind == hir_exp_num) {
        switch (op) {
        case hir_exp_op_bool_not:
            if (p_src_1->basic == type_int) p_src_1->intconst = !p_src_1->intconst;
            else if (p_src_1->basic == type_float) p_src_1->intconst = !p_src_1->floatconst;
            p_src_1->basic = type_int;
            break;
        case hir_exp_op_minus:
            if (p_src_1->basic == type_int) p_src_1->intconst = -p_src_1->intconst;
            else if (p_src_1->basic == type_float) p_src_1->floatconst = -p_src_1->floatconst;
            break;
        default:
            assert(1);
        }
        return p_src_1;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = NULL,
        .basic = type,
    };
    return p_exp;
}

static inline bool param_arr_check(p_symbol_type p_type_1, p_symbol_type p_type_2) {
    while (p_type_1->kind == type_arrary && p_type_2->kind == type_arrary) {
        p_type_2 = p_type_2->p_item;
        p_type_1 = p_type_1->p_item;
        if (p_type_1->size != p_type_2->size) return false;
    }
    return (p_type_2->kind == type_var && p_type_1->kind == type_var);
}

p_hir_exp hir_exp_call_gen(p_symbol_item p_item, p_hir_param_list p_param_list) {
    assert(p_item);
    assert(p_item->p_info->p_type->kind >= type_func);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_call,
        .p_func = p_item->p_func,
        .p_param_list = p_param_list,
    };

    p_symbol_type p_param_type = p_item->p_info->p_type->p_params;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param) {
        if (!p_param_type) {
            assert( p_item->p_info->p_type->kind == type_va_func);
            break;
        }

        p_hir_exp p_param = list_entry(p_node, hir_param, node)->p_exp;
        if (p_param_type->p_item->kind == type_arrary) {
            assert(p_param->kind == hir_exp_val);
            assert(param_arr_check(p_param_type->p_item, p_param->p_type));
        }
        else {
            assert(hir_exp_get_basic(p_param) == p_param_type->p_item->basic);
        }
        p_param_type = p_param_type->p_params;
    }
    assert(!p_param_type);
    return p_exp;
}

p_hir_exp hir_exp_val_gen(p_symbol_sym p_sym) {
    assert(p_sym);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp){
        .kind = hir_exp_val,
        .p_sym = p_sym,
        .p_offset = NULL,
        .p_type = p_sym->p_type,
    };
    return exp_val_const(p_exp);
}
p_hir_exp hir_exp_val_offset(p_hir_exp p_val, p_hir_exp p_offset) {
    assert(p_val->p_type->kind == type_arrary);
    p_hir_exp p_length = hir_exp_int_gen(p_val->p_type->p_item->size);
    p_offset = hir_exp_exec_gen(hir_exp_op_mul, p_offset, p_length);
    if (p_val->p_offset) {
        p_offset = hir_exp_exec_gen(hir_exp_op_add, p_val->p_offset, p_offset);
    }
    
    p_val->p_offset = p_offset;
    p_val->p_type = p_val->p_type->p_item;
    return exp_val_const(p_val);
}

p_hir_exp hir_exp_int_gen(INTCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .intconst = num,
        .basic = type_int,
    };
    return p_exp;
}
p_hir_exp hir_exp_float_gen(FLOATCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .floatconst = num,
        .basic = type_float,
    };
    return p_exp;
}

p_hir_exp hir_exp_str_gen(p_symbol_str p_str) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_str,
        .p_str = p_str,
    };
    return p_exp;
}

void hir_exp_drop(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_exec:
        switch (p_exp->op) {
        case hir_exp_op_assign:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
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
        case hir_exp_op_bool_and:
            hir_exp_drop(p_exp->p_src_1);
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_or:
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
        case hir_exp_op_bool_not:
            hir_exp_drop(p_exp->p_src_1);
            break;
        case hir_exp_op_minus:
            hir_exp_drop(p_exp->p_src_1);
            break;
        }
        break;
    case hir_exp_call:
        hir_param_list_drop(p_exp->p_param_list);
        break;
    case hir_exp_val:
        if (p_exp->p_offset) {
            hir_exp_drop(p_exp->p_offset);
        }
        break;
    case hir_exp_num:
        break;
    case hir_exp_str:
        break;
    }
    free(p_exp);
}