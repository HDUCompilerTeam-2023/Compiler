#include <frontend/syntax/decl/def.h>

#include <ast_gen/exp.h>
#include <frontend/syntax/type/gen.h>

const char *syntax_decl_list_node_get_name(p_syntax_decl p_decl) {
    return p_decl->name;
}
p_syntax_type_array syntax_decl_list_node_get_array(p_syntax_decl p_decl) {
    return p_decl->p_array;
}
p_syntax_init syntax_decl_list_node_get_init(p_syntax_decl p_decl) {
    return p_decl->p_init;
}
p_syntax_decl syntax_decl_arr(p_syntax_decl p_decl, p_ast_exp p_exp) {
    size_t size = 0;
    if (p_exp) { // TODO
        p_exp = ast_exp_ptr_to_val_check_basic(p_exp);
        assert(p_exp->p_type->basic == type_i32);
        assert(p_exp->i32const > 0);
        size = p_exp->i32const;
        ast_exp_drop(p_exp);
    }
    p_syntax_type_array p_arrary = syntax_type_array_gen(size);
    p_decl->p_array = syntax_type_add_array(p_decl->p_array, p_arrary);

    return p_decl;
}
p_syntax_decl syntax_decl_init(p_syntax_decl p_decl, p_syntax_init p_init) {
    p_decl->p_init = p_init;
    return p_decl;
}
