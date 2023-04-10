#include <hir_print.h>
#include <stdio.h>

#include <hir/program.h>
#include <hir/func.h>

int deep = 0;

void hir_program_print(p_hir_program p_program) {
    symbol_store_print(p_program->pss);
}
