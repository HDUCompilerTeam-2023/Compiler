#include <hir_print.h>
#include <stdio.h>

#include <hir/stmt.h>

#include <symbol_print.h>

void hir_stmt_print(p_hir_stmt p_stmt) {
    assert(p_stmt);
    switch (p_stmt->type) {
    case hir_stmt_block:
        hir_block_print(p_stmt->p_block);
        break;
    case hir_stmt_exp:
        printf("%*s", deep, "");
        if (p_stmt->p_exp) hir_exp_print(p_stmt->p_exp);
        printf(";\n");
        break;
    case hir_stmt_return:
        printf("%*sreturn ", deep, "");
        if (p_stmt->p_exp) hir_exp_print(p_stmt->p_exp);
        printf(";\n");
        break;
    case hir_stmt_if_else:
        printf("%*sif(", deep, "");
        hir_exp_print(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_print(p_stmt->p_stmt_1);
        deep -= 4;
        printf("%*selse\n", deep, "");
        deep += 4;
        hir_stmt_print(p_stmt->p_stmt_2);
        deep -= 4;
        break;
    case hir_stmt_while:
        printf("%*swhile(", deep, "");
        hir_exp_print(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_print(p_stmt->p_stmt_1);
        deep -= 4;
        break;
    case hir_stmt_if:
        printf("%*sif(", deep, "");
        hir_exp_print(p_stmt->p_exp);
        printf(")\n");
        deep += 4;
        hir_stmt_print(p_stmt->p_stmt_1);
        deep -= 4;
        break;
    case hir_stmt_break:
        printf("%*sbreak;\n", deep, "");
        break;
    case hir_stmt_continue:
        printf("%*scontinue;\n", deep, "");
        break;
    }
}
