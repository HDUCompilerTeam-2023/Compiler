#include <mir/vreg.h>
#include <mir_print.h>
#include <symbol_print.h>

#include <stdio.h>

void mir_vreg_print(p_mir_vreg p_vreg) {
    assert(p_vreg);
    symbol_type_print(p_vreg->p_type);
    printf(" %%%ld", p_vreg->id);
}
