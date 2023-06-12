#ifndef __AST_BLOCK__
#define __AST_BLOCK__

#include <ast.h>

struct ast_block {
    uint64_t length;
    list_head stmt;
};

#endif
