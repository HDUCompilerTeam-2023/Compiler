#include <hir_gen/stmt.h>

#include <hir_gen.h>


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
    if (p_block->length == 0) {
        free(p_block);
        return hir_stmt_exp_gen(NULL);
    }
    if (p_block->length == 1) {
        p_hir_stmt p_stmt = list_entry(p_block->stmt.p_next, hir_stmt, node);
        list_del(&p_stmt->node);
        free(p_block);
        return p_stmt;
    }
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
        if (p_stmt->p_exp) hir_exp_drop(p_stmt->p_exp);
        break;
    case hir_stmt_return:
        if (p_stmt->p_exp) hir_exp_drop(p_stmt->p_exp);
        break;
    case hir_stmt_if_else:
        hir_exp_drop(p_stmt->p_exp);
        hir_stmt_drop(p_stmt->p_stmt_1);
        hir_stmt_drop(p_stmt->p_stmt_2);
        break;
    case hir_stmt_while:
        hir_exp_drop(p_stmt->p_exp);
        hir_stmt_drop(p_stmt->p_stmt_1);
        break;
    case hir_stmt_if:
        hir_exp_drop(p_stmt->p_exp);
        hir_stmt_drop(p_stmt->p_stmt_1);
        break;
    case hir_stmt_break:
    case hir_stmt_continue:
        break;
    }
    free(p_stmt);
}
