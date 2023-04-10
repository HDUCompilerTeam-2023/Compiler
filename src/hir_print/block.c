#include <hir_print.h>
#include <stdio.h>

#include <hir/block.h>
#include <hir/stmt.h>

void hir_block_print(p_hir_block p_block) {
    assert(p_block);
    printf("%*s{\n", deep, "");
    deep += 4;
    p_list_head p_node;
    list_for_each(p_node, &p_block->stmt) {
        p_hir_stmt p_stmt = list_entry(p_node, hir_stmt, node);
        hir_stmt_print(p_stmt);
    }
    deep -= 4;
    printf("%*s}\n", deep, "");
}