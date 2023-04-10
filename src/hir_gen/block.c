#include <hir_gen/block.h>

#include <hir_gen.h>

p_hir_block hir_block_gen(void) {
    p_hir_block p_block = malloc(sizeof(*p_block));
    *p_block = (hir_block) {
        .length = 0,
        .stmt = list_head_init(&p_block->stmt),
    };
    return p_block;
}

p_hir_block hir_block_add(p_hir_block p_block, p_hir_stmt p_stmt) {
    if (p_stmt->type != hir_stmt_block) {
        if (p_stmt->type == hir_stmt_exp && p_stmt->p_exp == NULL) {
            free(p_stmt);
        }
        else {
            ++p_block->length;
            list_add_prev(&p_stmt->node, &p_block->stmt);
        }
    }
    else {
        p_block->length += p_stmt->p_block->length;
        list_blk_add_prev(&p_stmt->p_block->stmt, &p_block->stmt);
        free(p_stmt->p_block);
        free(p_stmt);
    }
    return p_block;
}

void hir_block_drop(p_hir_block p_block) {
    assert(p_block);
    while(!list_head_alone(&p_block->stmt)) {
        p_hir_stmt p_stmt = list_entry(p_block->stmt.p_next, hir_stmt, node);
        list_del(&p_stmt->node);
        hir_stmt_drop(p_stmt);
    }
    free(p_block);
}