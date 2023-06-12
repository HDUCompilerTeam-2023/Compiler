#include <ir/vreg.h>
#include <ir_print.h>
#include <symbol_print.h>

#include <stdio.h>

void ir_vreg_print(p_ir_vreg p_vreg) {
    assert(p_vreg);
    symbol_type_print(p_vreg->p_type);
    printf(" %%%ld", p_vreg->id);
}
