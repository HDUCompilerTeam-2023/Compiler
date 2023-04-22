#include <hir2mir.h>
#include <hir/block.h>
#include <hir/stmt.h>
// 生成函数的函数体
p_mir_basic_block hir2mir_block_gen(p_hir2mir_info p_info, p_hir_block p_block)
{
    assert(p_block);
    p_mir_basic_block p_entry = hir2mir_basic_block_gen(p_info);
    p_entry->block_id = p_info->block_id ++;
    p_info->p_current_basic_block = p_entry;
    p_list_head p_node;
    list_for_each(p_node, &p_block->stmt){
        p_hir_stmt p_stmt = list_entry(p_node, hir_stmt, node);
        hir2mir_stmt_gen(p_info, NULL, NULL, p_stmt);
    }
    return p_entry;
}

// 生成 空 basic_block 并将其添加到 函数的 basic_block_list 中
p_mir_basic_block hir2mir_basic_block_gen(p_hir2mir_info p_info)
{
    p_mir_basic_block p_new_basic_block = mir_basic_block_gen();
    mir_basic_block_list_add(p_info->p_basic_block_list, p_new_basic_block);
    return p_new_basic_block;
}