#include <ast_gen/exp.h>
#include <symbol/type.h>

#include <ast_gen/stmt.h>
#include <frontend/syntax/info/def.h>
#include <symbol/func.h>

p_ast_exp syntax_val_offset(p_ast_exp p_val, p_ast_exp p_offset) {
    if (p_val->p_type->ref_level > 1) {
        p_val = ast_exp_load_gen(p_val);
        return ast_exp_gep_gen(p_val, p_offset, false);
    }
    else {
        return ast_exp_gep_gen(p_val, p_offset, true);
    }
}

p_ast_stmt syntax_return(p_syntax_info p_info, p_ast_exp p_exp) {
    return ast_stmt_return_gen(p_info->p_func->ret_type, p_exp);
}
