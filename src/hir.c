#include <hir.h>
#include <stdio.h>

static int deep = 0;

p_hir_stmt hir_stmt_return_gen(p_hir_exp p_exp) {
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_return,
        .p_exp = p_exp,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_exp_gen(p_hir_exp p_exp) {
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_exp,
        .p_exp = p_exp,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_break_gen(void) {
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_break,
        .null = NULL,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_continue_gen(void) {
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_continue,
        .null = NULL,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_if_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1) {
    assert(p_exp && p_stmt_1);
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_if,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_if_else_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1, p_hir_stmt p_stmt_2) {
    assert(p_exp && p_stmt_1 && p_stmt_2);
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_if_else,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
        .p_stmt_2 = p_stmt_2,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_while_gen(p_hir_exp p_exp, p_hir_stmt p_stmt_1) {
    assert(p_exp && p_stmt_1);
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_while,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
    };
    return p_stmt;
}
p_hir_stmt hir_stmt_block_gen(p_hir_block p_block) {
    assert(p_block);
    p_hir_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (hir_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = hir_stmt_block,
        .p_block = p_block,
    };
    return p_stmt;
}
void hir_stmt_drop(p_hir_stmt p_stmt) {
    assert(p_stmt);
    switch (p_stmt->type) {
    case hir_stmt_block:
        hir_block_drop(p_stmt->p_block);
        break;
    case hir_stmt_exp:
        printf("%*s", deep, "");
        if (p_stmt->p_exp) hir_exp_drop(p_stmt->p_exp);
        printf(";\n");
        break;
    case hir_stmt_return:
        printf("%*sreturn ", deep, "");
        if (p_stmt->p_exp) hir_exp_drop(p_stmt->p_exp);
        printf(";\n");
        break;
    case hir_stmt_if_else:
        printf("%*sif(", deep, "");
        hir_exp_drop(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_drop(p_stmt->p_stmt_1);
        deep -= 4;
        printf("%*selse\n", deep, "");
        deep += 4;
        hir_stmt_drop(p_stmt->p_stmt_2);
        deep -= 4;
        break;
    case hir_stmt_while:
        printf("%*swhile(", deep, "");
        hir_exp_drop(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_drop(p_stmt->p_stmt_1);
        deep -= 4;
        break;
    case hir_stmt_if:
        printf("%*sif(", deep, "");
        hir_exp_drop(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_drop(p_stmt->p_stmt_1);
        deep -= 4;
        break;
    case hir_stmt_break:
        printf("%*sbreak;\n", deep, "");
        break;
    case hir_stmt_continue:
        printf("%*scontinue;\n", deep, "");
        break;
    }
    free(p_stmt);
}

static inline void hir_exp_can_exec(p_hir_exp p_exp) {
    assert(p_exp->is_basic);
    assert(p_exp->basic != type_void);
}

p_hir_exp hir_exp_trans_int2float(p_hir_exp p_int) {
    hir_exp_can_exec(p_int);
    p_hir_exp p_float = malloc(sizeof(*p_float));
    *p_float = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_int2float,
        .p_src_1 = p_int,
        .basic = type_float,
        .is_basic = true,
        .syntax_const_exp = p_int->syntax_const_exp,
    };
    return p_float;
}
p_hir_exp hir_exp_trans_float2int(p_hir_exp p_float) {
    hir_exp_can_exec(p_float);
    p_hir_exp p_int = malloc(sizeof(*p_int));
    *p_int = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_float2int,
        .p_src_1 = p_float,
        .basic = type_float,
        .is_basic = true,
        .syntax_const_exp = p_float->syntax_const_exp,
    };
    return p_int;
}

p_hir_exp hir_exp_dot_gen(p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_dot,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .is_basic = p_src_2->is_basic,
        .syntax_const_exp = false, // TODO ?
    };
    if (p_exp->is_basic) {
        p_exp->basic = p_src_2->basic;
    }
    else {
        p_exp->p_type = p_src_2->p_type;
    }
    return p_exp;
}
p_hir_exp hir_exp_assign_gen(p_hir_exp lval, p_hir_exp rval) {
    assert(lval && rval);
    hir_exp_can_exec(lval);
    hir_exp_can_exec(rval);

    basic_type type = lval->basic;
    if (rval->basic != type) {
        if (rval->basic == type_float) {
            rval = hir_exp_trans_int2float(rval);
        }
        else {
            rval = hir_exp_trans_int2float(rval);
        }
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_assign,
        .p_src_1 = lval,
        .p_src_2 = rval,
        .basic = type,
        .is_basic = true,
        .syntax_const_exp = false, // TODO ?
    };
    return p_exp;
}
p_hir_exp hir_exp_exec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    hir_exp_can_exec(p_src_1);
    hir_exp_can_exec(p_src_2);

    basic_type type = p_src_1->basic;
    if (p_src_2->basic != type) {
        type = type_float;
        if (p_src_2->basic == type_float) {
            p_src_1 = hir_exp_trans_int2float(p_src_1);
        }
        else {
            p_src_2 = hir_exp_trans_int2float(p_src_2);
        }
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type,
        .is_basic = true,
        .syntax_const_exp = p_src_1->syntax_const_exp && p_src_2->syntax_const_exp,
    };
    return p_exp;
}
p_hir_exp hir_exp_lexec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    hir_exp_can_exec(p_src_1);
    hir_exp_can_exec(p_src_2);

    basic_type type = type_int;

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type,
        .is_basic = true,
        .syntax_const_exp = p_src_1->syntax_const_exp && p_src_2->syntax_const_exp,
    };
    return p_exp;
}
p_hir_exp hir_exp_uexec_gen(hir_exp_op op, p_hir_exp p_src_1) {
    assert(p_src_1);
    hir_exp_can_exec(p_src_1);

    basic_type type = p_src_1->basic;
    if (op == hir_exp_op_bool_not) {
        type = type_int;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = NULL,
        .basic = type,
        .is_basic = true,
        .syntax_const_exp = p_src_1->syntax_const_exp,
    };
    return p_exp;
}

p_hir_exp hir_exp_call_gen(p_symbol_sym p_sym, p_hir_param_list p_param_list) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_call,
        .p_sym = p_sym,
        .p_param_list = p_param_list,
        .basic = p_sym->p_type->basic,
        .is_basic = true,
        .syntax_const_exp = false,
    };
    return p_exp;
}
p_hir_exp hir_exp_id_gen(p_symbol_sym p_sym) {
    assert(p_sym);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_id,
        .p_sym = p_sym,
        .p_type = p_sym->p_type,
        .is_basic = false,
        .syntax_const_exp = false,
    };
    assert(p_exp->p_type);
    if (p_exp->p_type->kind == type_var) {
        p_exp->is_basic = true;
        p_exp->basic = p_exp->p_type->basic;
    }
    return p_exp;
}
p_hir_exp hir_exp_arr_gen(p_hir_exp p_arrary, p_hir_exp p_index) {
    assert(!p_arrary->is_basic);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_arr,
        .p_src_1 = p_arrary,
        .p_src_2 = p_index,
        .p_type = p_arrary->p_type->p_item,
        .is_basic = false,
        .syntax_const_exp = false,
    };
    assert(p_exp->p_type);
    if (p_exp->p_type->kind == type_var) {
        p_exp->basic = p_exp->p_type->basic;
        p_exp->is_basic = true;
    }
    return p_exp;
}

p_hir_exp hir_exp_int_gen(INTCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .intconst = num,
        .basic = type_int,
        .is_basic = true,
        .syntax_const_exp = true,
    };
    return p_exp;
}
p_hir_exp hir_exp_float_gen(FLOATCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .floatconst = num,
        .basic = type_float,
        .is_basic = true,
        .syntax_const_exp = true,
    };
    return p_exp;
}

void hir_exp_drop(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_exec:
        switch (p_exp->op) {
        case hir_exp_op_assign:
            if (p_exp->basic == type_float) printf("f");
            printf("= ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_dot:
            if (p_exp->basic == type_float) printf("f");
            printf(", ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_add:
            if (p_exp->basic == type_float) printf("f");
            printf("+ ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_sub:
            if (p_exp->basic == type_float) printf("f");
            printf("- ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_mul:
            if (p_exp->basic == type_float) printf("f");
            printf("* ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_div:
            if (p_exp->basic == type_float) printf("f");
            printf("/ ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_mod:
            if (p_exp->basic == type_float) printf("f");
            printf("%% ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_and:
            if (p_exp->basic == type_float) printf("f");
            printf("&& ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_or:
            if (p_exp->basic == type_float) printf("f");
            printf("|| ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_eq:
            if (p_exp->basic == type_float) printf("f");
            printf("== ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_neq:
            if (p_exp->basic == type_float) printf("f");
            printf("!= ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_l:
            if (p_exp->basic == type_float) printf("f");
            printf("< ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_leq:
            if (p_exp->basic == type_float) printf("f");
            printf("<= ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_g:
            if (p_exp->basic == type_float) printf("f");
            printf("> ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_geq:
            if (p_exp->basic == type_float) printf("f");
            printf(">= ");
            hir_exp_drop(p_exp->p_src_1);
            printf(" ");
            hir_exp_drop(p_exp->p_src_2);
            break;
        case hir_exp_op_bool_not:
            if (p_exp->basic == type_float) printf("f");
            printf("! ");
            hir_exp_drop(p_exp->p_src_1);
            break;
        case hir_exp_op_minus:
            if (p_exp->basic == type_float) printf("f");
            printf("minus ");
            hir_exp_drop(p_exp->p_src_1);
            break;
        case hir_exp_op_int2float:
            printf("int2float ");
            hir_exp_drop(p_exp->p_src_1);
            break;
        case hir_exp_op_float2int:
            printf("float2int ");
            hir_exp_drop(p_exp->p_src_1);
            break;
        case hir_exp_op_arr:
            hir_exp_drop(p_exp->p_src_1);
            printf("[");
            hir_exp_drop(p_exp->p_src_2);
            printf("]");
            break;
        }
        break;
    case hir_exp_call:
        printf("%s(", p_exp->p_sym->name);
        hir_param_list_drop(p_exp->p_param_list);
        printf(")");
        break;
    case hir_exp_id:
        printf("%s", p_exp->p_sym->name);
        break;
    case hir_exp_num:
        if (p_exp->basic == type_float) printf("%lf", p_exp->floatconst);
        else printf("%ld", p_exp->intconst);
        break;
    }
    free(p_exp);
}


p_hir_param_list hir_param_list_init(void) {
    p_hir_param_list p_param_list = malloc(sizeof(*p_param_list));
    *p_param_list = (hir_param_list) {
        .param = list_head_init(&p_param_list->param),
    };
    return p_param_list;
}
p_hir_param_list hir_param_list_add(p_hir_param_list p_param_list, p_hir_exp p_exp) {
    p_hir_param p_param = malloc(sizeof(*p_param));
    *p_param = (hir_param) {
        .p_exp = p_exp,
        .node = list_head_init(&p_param->node),
    };

    list_add_prev(&p_param->node, &p_param_list->param);
    return p_param_list;
}
void hir_param_drop (p_hir_param p_param) {
    assert(p_param);
    hir_exp_drop(p_param->p_exp);
    free(p_param);
}
void hir_param_list_drop(p_hir_param_list p_param_list) {
    assert(p_param_list);
    bool is_first = true;
    while(!list_head_alone(&p_param_list->param)) {
        if (!is_first) printf(", ");
        else is_first = false;
        p_hir_param p_param = list_entry(p_param_list->param.p_next, hir_param, node);
        list_del(&p_param->node);
        hir_param_drop(p_param);
    }
    free(p_param_list);
}

p_hir_block hir_block_gen(void) {
    p_hir_block p_block = malloc(sizeof(*p_block));
    *p_block = (hir_block) {
        .stmt = list_head_init(&p_block->stmt),
    };
    return p_block;
}
p_hir_block hir_block_add(p_hir_block p_block, p_hir_stmt p_stmt) {
    list_add_prev(&p_stmt->node, &p_block->stmt);
    return p_block;
}
void hir_block_drop(p_hir_block p_block) {
    assert(p_block);
    printf("%*s{\n", deep, "");
    deep += 4;
    while(!list_head_alone(&p_block->stmt)) {
        p_hir_stmt p_stmt = list_entry(p_block->stmt.p_next, hir_stmt, node);
        list_del(&p_stmt->node);
        hir_stmt_drop(p_stmt);
    }
    deep -= 4;
    printf("%*s}\n", deep, "");
    free(p_block);
}

p_hir_func hir_func_gen(p_symbol_sym p_sym, p_hir_block p_block) {
    p_hir_func p_func = malloc(sizeof(*p_func));
    *p_func = (hir_func) {
        .p_sym = p_sym,
        .p_block = p_block,
        .node = list_head_init(&p_func->node),
    };
    return p_func;
}
void hir_func_drop(p_hir_func p_func) {
    assert(p_func);

    if (p_func->p_sym->p_type->basic == type_int) printf("int ");
    else if (p_func->p_sym->p_type->basic == type_float) printf("float ");
    printf("%s()\n", p_func->p_sym->name);
    hir_block_drop(p_func->p_block);
    free(p_func);
}

p_hir_program hir_program_gen(void) {
    p_hir_program p_program = malloc(sizeof(*p_program));
    *p_program = (hir_program) {
        .func = list_head_init(&p_program->func),
        .pss = symbol_store_initial(),
    };
    return p_program;
}
p_hir_program hir_program_add(p_hir_program p_program, p_hir_func p_func){
    list_add_prev(&p_func->node, &p_program->func);
    return p_program;
}
void hir_program_drop(p_hir_program p_program) {
    assert(p_program);
    while(!list_head_alone(&p_program->func)) {
        p_hir_func p_func = list_entry(p_program->func.p_next, hir_func, node);
        list_del(&p_func->node);
        hir_func_drop(p_func);
    }
    symbol_store_destroy(p_program->pss);
    free(p_program);
}