#ifndef __AST_GEN_BLOCK__
#define __AST_GEN_BLOCK__

#include <ast/block.h>

p_ast_block ast_block_gen(void);
p_ast_block ast_block_add(p_ast_block p_block, p_ast_stmt p_stmt);
void ast_block_drop(p_ast_block p_block);

#endif
