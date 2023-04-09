#include <hir_gen/exp.h>

#include <hir_gen.h>

static inline basic_type exp_basic(p_hir_exp p_exp) {
  if (p_exp->kind == hir_exp_call) {
    assert(p_exp->p_type->kind == type_func);
    return p_exp->p_type->basic;
  } else if (p_exp->kind == hir_exp_val) {
    assert(p_exp->p_type->kind == type_var);
    return p_exp->p_type->basic;
  }
  return p_exp->basic;
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
        .syntax_const_exp = false, // TODO ?
    };
    return p_exp;
}
p_hir_exp hir_exp_exec_gen(hir_exp_op op, p_hir_exp p_src_1, p_hir_exp p_src_2) {
    assert(p_src_1 && p_src_2);
    assert(exp_basic(p_src_1) != type_void);
    assert(exp_basic(p_src_2) != type_void);
    if (op == hir_exp_op_mod) assert(exp_basic(p_src_1) == type_int && exp_basic(p_src_1) == type_int);

    basic_type type = exp_basic(p_src_1);
    if (exp_basic(p_src_1) != type) {
        type = type_float;
    }

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = p_src_2,
        .basic = type,
        .syntax_const_exp = p_src_1->syntax_const_exp && p_src_2->syntax_const_exp,
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
        .syntax_const_exp = p_src_1->syntax_const_exp && p_src_2->syntax_const_exp,
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

    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_exec,
        .op = op,
        .p_src_1 = p_src_1,
        .p_src_2 = NULL,
        .basic = type,
        .syntax_const_exp = p_src_1->syntax_const_exp,
    };
    return p_exp;
}

p_hir_exp hir_exp_call_gen(p_symbol_sym p_sym, p_hir_param_list p_param_list) {
    assert(p_sym);
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_call,
        .p_sym = p_sym,
        .p_param_list = p_param_list,
        .p_type = p_sym->p_type,
        .syntax_const_exp = false,
    };
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
        .syntax_const_exp = false,
    };
    return p_exp;
}
p_hir_exp hir_exp_val_offset(p_hir_exp p_val, p_hir_exp p_offset) {
    assert(p_val->p_type->kind == type_arrary);
    uint64_t size = p_val->p_type->size;
    p_val->p_type = p_val->p_type->p_item;
    if (p_val->p_offset) {
        p_hir_exp p_length = hir_exp_int_gen(size);
        p_val->p_offset = hir_exp_exec_gen(hir_exp_op_mul, p_val->p_offset, p_length);
        p_val->p_offset = hir_exp_exec_gen(hir_exp_op_add, p_val->p_offset, p_offset);
    }
    else {
        p_val->p_offset = p_offset;
    }
    return p_val;
}

p_hir_exp hir_exp_int_gen(INTCONST_t num) {
    p_hir_exp p_exp = malloc(sizeof(*p_exp));
    *p_exp = (hir_exp) {
        .kind = hir_exp_num,
        .intconst = num,
        .basic = type_int,
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
        }
        break;
    case hir_exp_call:
        printf("%s(", p_exp->p_sym->name);
        hir_param_list_drop(p_exp->p_param_list);
        printf(")");
        break;
    case hir_exp_val:
        printf("%s", p_exp->p_sym->name);
        if (p_exp->p_offset) {
            printf("[");
            hir_exp_drop(p_exp->p_offset);
            printf("]");
        }
        break;
    case hir_exp_num:
        if (p_exp->basic == type_float) printf("%lf", p_exp->floatconst);
        else printf("%ld", p_exp->intconst);
        break;
    }
    free(p_exp);
}