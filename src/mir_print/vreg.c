#include <mir/vreg.h>
#include <mir_print.h>

#include <stdio.h>

void mir_vreg_print(p_mir_vreg p_vreg) {
    assert(p_vreg);
    mir_basic_type_print(p_vreg->b_type);
    for (size_t i = 0; i < p_vreg->ref_level; ++i) {
        printf("*");
    }
    printf(" %%%ld", p_vreg->id);
}
