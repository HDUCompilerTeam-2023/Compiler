#include <hir_gen/exp.h>

#include <hir_gen.h>

static inline void hir_exp_can_exec(p_hir_exp p_exp) {
    assert(p_exp->is_basic);
    assert(p_exp->basic != type_void);
}

p_hir_exp hir_exp_assign_gen(p_hir_exp lval, p_hir_exp rval) {
    assert(lval && rval);
    hir_exp_can_exec(lval);
    hir_exp_can_exec(rval);

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = hir_exp_op_assign,
        .p_src_1 = lval,
        .p_src_2 = rval,
        .basic = lval->basic,
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

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type_int,
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

#include <stdio.h>
void hir_exp_drop(p_hir_exp p_exp) {
    assert(p_exp);
    switch (p_exp->kind) {
    case hir_exp_exec:
        switch (p_exp->op) {
        case hir_exp_op_assign:
            hir_exp_drop(p_exp->p_src_1);
            printf(" = ");
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