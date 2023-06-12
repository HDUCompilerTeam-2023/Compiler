#ifndef __AST_STMT__
#define __AST_STMT__

#include <ast.h>

struct ast_stmt {
    enum {
        ast_stmt_assign,
        ast_stmt_return,
        ast_stmt_exp, // p_exp (can null)
        ast_stmt_block, // p_block
        ast_stmt_if_else, // p_exp p_stmt_1 p_stmt_2
        ast_stmt_if,
        ast_stmt_while, // p_exp p_stmt_1
        ast_stmt_break,
        ast_stmt_continue, // null
    } type;

    union {
        struct {
            p_ast_exp p_exp;
            p_ast_stmt p_stmt_1, p_stmt_2;
        };
        struct {
            p_ast_exp p_lval, p_rval;
        };
        p_ast_block p_block;
        void *null;
    };

    list_head node;
};

#endif
