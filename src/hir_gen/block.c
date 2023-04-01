#include <hir_gen/block.h>

#include <hir_gen.h>

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

#include <stdio.h>
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