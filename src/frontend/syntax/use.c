#include <ast_gen/exp.h>
#include <symbol/type.h>

p_ast_exp syntax_val_offset(p_ast_exp p_val, p_ast_exp p_offset) {
    if (p_val->p_type->ref_level > 1) {
        p_val = ast_exp_load_gen(p_val);
        return ast_exp_gep_gen(p_val, p_offset, false);
    }
    else {
        return ast_exp_gep_gen(p_val, p_offset, true);
    }
}
