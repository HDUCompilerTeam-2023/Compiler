#include <hir_gen/exp.h>
#include <symbol/type.h>

p_hir_exp syntax_val_offset(p_hir_exp p_val, p_hir_exp p_offset) {
    if (p_val->p_type->ref_level > 1) {
        p_val = hir_exp_load_gen(p_val);
        return hir_exp_gep_gen(p_val, p_offset, false);
    }
    else {
        return hir_exp_gep_gen(p_val, p_offset, true);
    }
}
