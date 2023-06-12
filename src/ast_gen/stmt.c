#include <ast_gen/stmt.h>

#include <ast_gen.h>

p_ast_stmt ast_stmt_return_gen(p_ast_exp p_exp) {
    if (p_exp)
        p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_return,
        .p_exp = p_exp,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_exp_gen(p_ast_exp p_exp) {
    if (p_exp)
        p_exp = ast_exp_ptr_to_val(p_exp);
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_exp,
        .p_exp = p_exp,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_break_gen(void) {
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_break,
        .null = NULL,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_continue_gen(void) {
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_continue,
        .null = NULL,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_if_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1) {
    assert(p_exp && p_stmt_1);
    p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_if,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_if_else_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1, p_ast_stmt p_stmt_2) {
    assert(p_exp && p_stmt_1 && p_stmt_2);
    p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_if_else,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
        .p_stmt_2 = p_stmt_2,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_while_gen(p_ast_exp p_exp, p_ast_stmt p_stmt_1) {
    assert(p_exp && p_stmt_1);
    p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_while,
        .p_exp = p_exp,
        .p_stmt_1 = p_stmt_1,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_block_gen(p_ast_block p_block) {
    assert(p_block);
    if (p_block->length == 0) {
        free(p_block);
        return ast_stmt_exp_gen(NULL);
    }
    if (p_block->length == 1) {
        p_ast_stmt p_stmt = list_entry(p_block->stmt.p_next, ast_stmt, node);
        list_del(&p_stmt->node);
        free(p_block);
        return p_stmt;
    }
    p_ast_stmt p_stmt = malloc(sizeof(*p_stmt));
    *p_stmt = (ast_stmt) {
        .node = list_head_init(&p_stmt->node),
        .type = ast_stmt_block,
        .p_block = p_block,
    };
    return p_stmt;
}
p_ast_stmt ast_stmt_assign_gen(p_ast_exp lval, p_ast_exp rval) {
    assert(lval && rval);
    ast_exp_ptr_check_lval(lval);
    rval = ast_exp_ptr_to_val_check_basic(rval);

    p_ast_stmt p_exp = malloc(sizeof(*p_exp));
    *p_exp = (ast_stmt) {
        .node = list_head_init(&p_exp->node),
        .type = ast_stmt_assign,
        .p_lval = lval,
        .p_rval = rval,
    };
    return p_exp;
}
void ast_stmt_drop(p_ast_stmt p_stmt) {
    assert(p_stmt);
    list_del(&p_stmt->node);
    switch (p_stmt->type) {
    case ast_stmt_assign:
        ast_exp_drop(p_stmt->p_lval);
        ast_exp_drop(p_stmt->p_rval);
        break;
    case ast_stmt_block:
        ast_block_drop(p_stmt->p_block);
        break;
    case ast_stmt_exp:
        if (p_stmt->p_exp) ast_exp_drop(p_stmt->p_exp);
        break;
    case ast_stmt_return:
        if (p_stmt->p_exp) ast_exp_drop(p_stmt->p_exp);
        break;
    case ast_stmt_if_else:
        ast_exp_drop(p_stmt->p_exp);
        ast_stmt_drop(p_stmt->p_stmt_1);
        ast_stmt_drop(p_stmt->p_stmt_2);
        break;
    case ast_stmt_while:
        ast_exp_drop(p_stmt->p_exp);
        ast_stmt_drop(p_stmt->p_stmt_1);
        break;
    case ast_stmt_if:
        ast_exp_drop(p_stmt->p_exp);
        ast_stmt_drop(p_stmt->p_stmt_1);
        break;
    case ast_stmt_break:
    case ast_stmt_continue:
        break;
    }
    free(p_stmt);
}
