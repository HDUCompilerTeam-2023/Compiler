#ifndef __HIR_STMT__
#define __HIR_STMT__

#include <hir.h>

struct hir_stmt {
    enum {
        hir_stmt_return, hir_stmt_exp, // p_exp (can null)
        hir_stmt_block, // p_block
        hir_stmt_if_else, // p_exp p_stmt_1 p_stmt_2
        hir_stmt_if, hir_stmt_while, // p_exp p_stmt_1
        hir_stmt_break, hir_stmt_continue, // null
    } type;

    union {
        struct {
            p_hir_exp p_exp;
            p_hir_stmt p_stmt_1, p_stmt_2;
        };
        p_hir_block p_block;
        void *null;
    };

    list_head node;
};

#endif