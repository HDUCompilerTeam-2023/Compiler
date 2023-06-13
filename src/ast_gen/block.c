#include <ast_gen/block.h>

#include <ast_gen.h>

p_ast_block ast_block_gen(void) {
    p_ast_block p_block = malloc(sizeof(*p_block));
    *p_block = (ast_block) {
        .length = 0,
        .stmt = list_head_init(&p_block->stmt),
    };
    return p_block;
}

p_ast_block ast_block_add(p_ast_block p_block, p_ast_stmt p_stmt) {
    if (p_stmt->type != ast_stmt_block) {
        if (p_stmt->type == ast_stmt_exp && p_stmt->p_exp == NULL) {
            free(p_stmt);
        }
        else {
            ++p_block->length;
            list_add_prev(&p_stmt->node, &p_block->stmt);
        }
    }
    else {
        p_block->length += p_stmt->p_block->length;
        if (!list_blk_add_prev(&p_stmt->p_block->stmt, &p_block->stmt))
            list_replace(&p_block->stmt, &p_stmt->p_block->stmt);
        free(p_stmt->p_block);
        free(p_stmt);
    }
    return p_block;
}

void ast_block_drop(p_ast_block p_block) {
    assert(p_block);
    while (!list_head_alone(&p_block->stmt)) {
        p_ast_stmt p_stmt = list_entry(p_block->stmt.p_next, ast_stmt, node);
        ast_stmt_drop(p_stmt);
    }
    free(p_block);
}
