#ifndef __HIR_GEN_BLOCK__
#define __HIR_GEN_BLOCK__

#include <hir/block.h>

p_hir_block hir_block_gen(void);
p_hir_block hir_block_add(p_hir_block p_block, p_hir_stmt p_stmt);
void hir_block_drop(p_hir_block p_block);

#endif