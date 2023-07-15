#include <ast_gen/exp.h>

#include <ast_gen.h>

#include <symbol_gen.h>

static inline p_ast_exp exp_ptr_to_val(p_ast_exp p_exp) {
    if (p_exp->p_type->ref_level == 0) {
        assert(list_head_alone(&p_exp->p_type->array));
        return p_exp;
    }
    if (p_exp->p_type->ref_level > 1) {
        return ast_exp_load_gen(p_exp);
    }
    // assert(p_exp->p_type->ref_level == 1);
    if (list_head_alone(&p_exp->p_type->array)) {
        return ast_exp_load_gen(p_exp);
    }
    return ast_exp_gep_gen(p_exp, ast_exp_int_gen(0), true);
}

static inline void exp_check_basic(p_ast_exp p_exp) {
    assert(list_head_alone(&p_exp->p_type->array) && p_exp->p_type->ref_level == 0);
    assert(p_exp->p_type->basic != type_void);
}

static inline p_ast_exp exp_ptr_to_val_check_basic(p_ast_exp p_exp) {
    p_exp = exp_ptr_to_val(p_exp);
    exp_check_basic(p_exp);
    return p_exp;
}

static inline p_ast_exp exp_val_const(p_ast_exp p_exp) {
    if (p_exp->p_type->ref_level != 0) return p_exp;
    assert(list_head_alone(&p_exp->p_type->array));

    p_ast_exp p_back = p_exp;
    size_t offset = 0;
    size_t length = 1;
    p_exp = p_exp->p_ptr;
    while (p_exp->kind == ast_exp_gep) {
        if (p_exp->p_offset->kind != ast_exp_num)
            break;
        if (p_exp->p_offset->p_type->basic != type_i32)
            break;
        length *= symbol_type_get_size(p_exp->p_type);
        offset += length * p_exp->p_offset->i32const;
        p_exp = p_exp->p_addr;
    }
    if (p_exp->kind == ast_exp_ptr && p_exp->p_var->is_const) {
        assert(offset < symbol_type_get_size(p_exp->p_var->p_type));
        p_ast_exp p_val;
        if (p_exp->p_type->basic == type_i32) {
            p_val = ast_exp_int_gen(p_exp->p_var->p_init->memory[offset].i);
        }
        else {
            assert(p_exp->p_var->p_init);
            p_val = ast_exp_float_gen(p_exp->p_var->p_init->memory[offset].f);
        }
        ast_exp_drop(p_back);

        return p_val;
    }
    return p_back;
}

void ast_exp_ptr_check_lval(p_ast_exp p_exp) {
    assert(list_head_alone(&p_exp->p_type->array));
    assert(p_exp->p_type->ref_level == 1);
    assert(p_exp->p_type->basic != type_void);
}
p_ast_exp ast_exp_ptr_check_const(p_ast_exp p_exp) {
    p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    assert(p_exp->kind == ast_exp_num);
    return p_exp;
}
p_ast_exp ast_exp_ptr_to_val_check_basic(p_ast_exp p_exp) {
    return exp_ptr_to_val_check_basic(p_exp);
}
p_ast_exp ast_exp_ptr_to_val(p_ast_exp p_exp) {
    return exp_ptr_to_val(p_exp);
}

static inline p_ast_exp ast_exp_i2f_gen(p_ast_exp p_i32) {
    assert(p_i32);
    p_i32 = exp_ptr_to_val_check_basic(p_i32);
    assert(p_i32->p_type->basic == type_i32);

    if (p_i32->kind == ast_exp_num) {
        p_i32->p_type->basic = type_f32;
        p_i32->f32const = p_i32->i32const;
        return p_i32;
    }

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_unary,
        .u_op = ast_exp_op_i2f,
        .p_src = p_i32,
        .p_type = symbol_type_var_gen(type_f32),
        .p_des = NULL,
    };
    return p_exp;
}

static inline p_ast_exp ast_exp_f2i_gen(p_ast_exp p_f32) {
    assert(p_f32);
    p_f32 = exp_ptr_to_val_check_basic(p_f32);
    assert(p_f32->p_type->basic == type_f32);

    if (p_f32->kind == ast_exp_num) {
        p_f32->p_type->basic = type_i32;
        p_f32->i32const = p_f32->f32const;
        return p_f32;
    }

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_unary,
        .u_op = ast_exp_op_f2i,
        .p_src = p_f32,
        .p_type = symbol_type_var_gen(type_i32),
        .p_des = NULL,
    };
    return p_exp;
}

p_ast_exp ast_exp_cov_gen(p_ast_exp p_exp, basic_type b_type) {
    exp_check_basic(p_exp);
    if (p_exp->p_type->basic != b_type) {
        if (p_exp->p_type->basic == type_i32) {
            return ast_exp_i2f_gen(p_exp);
        }
        else if (p_exp->p_type->basic == type_f32) {
            return ast_exp_f2i_gen(p_exp);
        }
    }
    return p_exp;
}

p_ast_exp ast_exp_to_cond(p_ast_exp p_exp) {
    p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    if (p_exp->kind == ast_exp_relational)
        return p_exp;
    if (p_exp->kind == ast_exp_logic)
        return p_exp;
    if (p_exp->kind == ast_exp_ulogic)
        return p_exp;
    return ast_exp_relational_gen(ast_exp_op_neq, p_exp, ast_exp_int_gen(0));
}

bool ast_exp_ptr_is_stack(p_ast_exp p_exp) {
    p_ast_exp p_tmp = p_exp;
    while (p_tmp->kind == ast_exp_use) {
        p_tmp = p_tmp->p_exp;
    }
    if (p_tmp->kind == ast_exp_gep) {
        return p_tmp->is_stack_for_gep;
    }
    if (p_tmp->kind == ast_exp_load) {
        return false;
    }
    assert(p_tmp->kind == ast_exp_ptr);
    if (p_tmp->p_var->is_global) return false;
    return true;
}

p_ast_exp ast_exp_use_gen(p_ast_exp p_used_exp) {
    assert(p_used_exp);
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_use,
        .p_exp = p_used_exp,
        .p_type = symbol_type_copy(p_used_exp->p_type),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_binary_gen(ast_exp_binary_op b_op, p_ast_exp p_src_1, p_ast_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    p_src_1 = exp_ptr_to_val_check_basic(p_src_1);
    p_src_2 = exp_ptr_to_val_check_basic(p_src_2);
    if (b_op == ast_exp_op_mod) assert(p_src_1->p_type->basic == type_i32 && p_src_2->p_type->basic == type_i32);

    if (p_src_1->p_type->basic == type_f32) {
        p_src_2 = ast_exp_cov_gen(p_src_2, type_f32);
    }
    else if (p_src_2->p_type->basic == type_f32) {
        p_src_1 = ast_exp_cov_gen(p_src_1, type_f32);
    }
    basic_type type = p_src_1->p_type->basic;

    if (p_src_1->kind == ast_exp_num && p_src_2->kind == ast_exp_num) {
        switch (b_op) {
        case ast_exp_op_add:
            if (type == type_i32) {
                p_src_1->i32const = p_src_1->i32const + p_src_2->i32const;
            }
            else if (type == type_f32) {
                p_src_1->f32const = p_src_1->f32const + p_src_2->f32const;
            }
            break;
        case ast_exp_op_sub:
            if (type == type_i32) {
                p_src_1->i32const = p_src_1->i32const - p_src_2->i32const;
            }
            else if (type == type_f32) {
                p_src_1->f32const = p_src_1->f32const - p_src_2->f32const;
            }
            break;
        case ast_exp_op_mul:
            if (type == type_i32) {
                p_src_1->i32const = p_src_1->i32const * p_src_2->i32const;
            }
            else if (type == type_f32) {
                p_src_1->f32const = p_src_1->f32const * p_src_2->f32const;
            }
            break;
        case ast_exp_op_div:
            if (type == type_i32) {
                p_src_1->i32const = p_src_1->i32const / p_src_2->i32const;
            }
            else if (type == type_f32) {
                p_src_1->f32const = p_src_1->f32const / p_src_2->f32const;
            }
            break;
        case ast_exp_op_mod:
            p_src_1->i32const = p_src_1->i32const % p_src_2->i32const;
            break;
        default:
            assert(1);
        }
        ast_exp_drop(p_src_2);
        return p_src_1;
    }

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_binary,
        .b_op = b_op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .p_type = symbol_type_var_gen(type),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_relational_gen(ast_exp_relational_op r_op, p_ast_exp p_rsrc_1, p_ast_exp p_rsrc_2) {
    assert(p_rsrc_1 && p_rsrc_2);
    p_rsrc_1 = exp_ptr_to_val_check_basic(p_rsrc_1);
    p_rsrc_2 = exp_ptr_to_val_check_basic(p_rsrc_2);

    if (p_rsrc_1->p_type->basic == type_f32) {
        p_rsrc_2 = ast_exp_cov_gen(p_rsrc_2, type_f32);
    }
    else if (p_rsrc_2->p_type->basic == type_f32) {
        p_rsrc_1 = ast_exp_cov_gen(p_rsrc_1, type_f32);
    }

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_relational,
        .r_op = r_op,
        .p_rsrc_1 = p_rsrc_1,
        .p_rsrc_2 = p_rsrc_2,
        .p_type = symbol_type_var_gen(type_i32),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_logic_gen(ast_exp_logic_op l_op, p_ast_exp p_bool_1, p_ast_exp p_bool_2) {
    assert(p_bool_1 && p_bool_2);
    p_bool_1 = exp_ptr_to_val_check_basic(p_bool_1);
    p_bool_2 = exp_ptr_to_val_check_basic(p_bool_2);

    p_bool_1 = ast_exp_to_cond(p_bool_1);
    p_bool_2 = ast_exp_to_cond(p_bool_2);

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_logic,
        .l_op = l_op,
        .p_bool_1 = p_bool_1,
        .p_bool_2 = p_bool_2,
        .p_type = symbol_type_var_gen(type_i32),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_ulogic_gen(ast_exp_ulogic_op ul_op, p_ast_exp p_bool) {
    assert(p_bool);
    p_bool = exp_ptr_to_val_check_basic(p_bool);

    p_bool = ast_exp_to_cond(p_bool);

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_ulogic,
        .ul_op = ul_op,
        .p_bool = p_bool,
        .p_type = symbol_type_var_gen(type_i32),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_unary_gen(ast_exp_unary_op u_op, p_ast_exp p_src) {
    assert(p_src);
    p_src = exp_ptr_to_val_check_basic(p_src);

    basic_type type = p_src->p_type->basic;

    if (p_src->kind == ast_exp_num) {
        switch (u_op) {
        case ast_exp_op_minus:
            if (p_src->p_type->basic == type_i32) p_src->i32const = -p_src->i32const;
            else if (p_src->p_type->basic == type_f32)
                p_src->f32const = -p_src->f32const;
            break;
        default:
            assert(1);
        }
        return p_src;
    }

    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_unary,
        .u_op = u_op,
        .p_src = p_src,
        .p_type = symbol_type_var_gen(type),
        .p_des = NULL,
    };
    return p_exp;
}

static inline bool param_arr_check(p_symbol_type p_type_f, p_symbol_type p_type_r) {
    if (p_type_f->ref_level != p_type_r->ref_level) return false;
    if (p_type_f->ref_level == 1 && p_type_f->basic == type_void) {
        return p_type_r->ref_level == 1;
    }
    if (p_type_f->basic != p_type_r->basic) return false;
    p_list_head p_node_f, p_node_r = p_type_r->array.p_prev;
    list_for_each_tail(p_node_f, &p_type_f->array) {
        if (p_node_r == &p_type_r->array) return false;
        p_symbol_type_array p_array_f, p_array_r;
        p_array_f = list_entry(p_node_f, symbol_type_array, node);
        p_array_r = list_entry(p_node_r, symbol_type_array, node);
        p_node_r = p_node_r->p_prev;
        if (symbol_type_array_get_size(p_array_f) == 0 || symbol_type_array_get_size(p_array_r) == 0) continue;
        if (symbol_type_array_get_size(p_array_f) != symbol_type_array_get_size(p_array_r)) return false;
    }
    return true;
}

p_ast_exp ast_exp_call_gen(p_symbol_func p_func, p_ast_param_list p_param_list) {
    assert(p_func);
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_call,
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

        p_ast_param p_param = list_entry(p_node, ast_param, node);
        p_ast_exp p_param_exp = p_param->p_exp;
        if (p_param_type->ref_level == 0 && list_head_alone(&p_param_type->array)) {
            p_param_exp = p_param->p_exp = ast_exp_cov_gen(p_param_exp, p_param_type->basic);
        }
        assert(param_arr_check(p_param_type, p_param_exp->p_type));
        p_node_Fparam = p_node_Fparam->p_next;
    }
    assert(p_node_Fparam == &p_func->param);
    return p_exp;
}

p_ast_exp ast_exp_ptr_gen(p_symbol_var p_var) {
    assert(p_var);
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_ptr,
        .p_var = p_var,
        .p_type = symbol_type_copy(p_var->p_type),
        .p_des = NULL,
    };
    symbol_type_push_ptr(p_exp->p_type);
    return p_exp;
}
p_ast_exp ast_exp_gep_gen(p_ast_exp p_val, p_ast_exp p_offset, bool is_element) {
    assert(p_val->p_type->ref_level == 1);
    assert(!is_element || !list_head_alone(&p_val->p_type->array));
    p_offset = exp_ptr_to_val_check_basic(p_offset);
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_gep,
        .is_stack_for_gep = ast_exp_ptr_is_stack(p_val),
        .p_addr = p_val,
        .p_offset = p_offset,
        .is_element = is_element,
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
p_ast_exp ast_exp_load_gen(p_ast_exp p_ptr) {
    assert(p_ptr->p_type->ref_level > 0);
    if (p_ptr->p_type->ref_level == 1) {
        assert(list_head_alone(&p_ptr->p_type->array));
    }
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_load,
        .is_stack = ast_exp_ptr_is_stack(p_ptr),
        .p_ptr = p_ptr,
        .p_type = symbol_type_copy(p_ptr->p_type),
        .p_des = NULL,
    };
    symbol_type_pop_ptr(p_exp->p_type);

    return exp_val_const(p_exp);
}

p_ast_exp ast_exp_int_gen(I32CONST_t num) {
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_num,
        .i32const = num,
        .p_type = symbol_type_var_gen(type_i32),
        .p_des = NULL,
    };
    return p_exp;
}
p_ast_exp ast_exp_float_gen(F32CONST_t num) {
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_num,
        .f32const = num,
        .p_type = symbol_type_var_gen(type_f32),
        .p_des = NULL,
    };
    return p_exp;
}

p_ast_exp ast_exp_str_gen(p_symbol_str p_str) {
    p_ast_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_exp) {
        .kind = ast_exp_num,
        .p_str = p_str,
        .p_type = symbol_type_var_gen(type_str),
        .p_des = NULL,
    };
    return p_exp;
}

void ast_exp_drop(p_ast_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case ast_exp_binary:
        ast_exp_drop(p_exp->p_src_1);
        ast_exp_drop(p_exp->p_src_2);
        break;
    case ast_exp_relational:
        ast_exp_drop(p_exp->p_rsrc_1);
        ast_exp_drop(p_exp->p_rsrc_2);
        break;
    case ast_exp_unary:
        ast_exp_drop(p_exp->p_src);
        break;
    case ast_exp_logic:
        ast_exp_drop(p_exp->p_bool_1);
        ast_exp_drop(p_exp->p_bool_2);
        break;
    case ast_exp_ulogic:
        ast_exp_drop(p_exp->p_bool);
        break;
    case ast_exp_call:
        ast_param_list_drop(p_exp->p_param_list);
        break;
    case ast_exp_gep:
        ast_exp_drop(p_exp->p_addr);
        ast_exp_drop(p_exp->p_offset);
        break;
    case ast_exp_load:
        ast_exp_drop(p_exp->p_ptr);
        break;
    case ast_exp_ptr:
    case ast_exp_num:
    case ast_exp_use:
        break;
    }
    symbol_type_drop(p_exp->p_type);
    free(p_exp);
}
