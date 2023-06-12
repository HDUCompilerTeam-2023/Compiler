#include <frontend/syntax/decl_list/node/def.h>

#include <ast_gen/exp.h>
#include <frontend/syntax/type/gen.h>

const char *syntax_decl_list_node_get_name(p_syntax_decl p_decl) {
    return p_decl->name;
}
basic_type syntax_decl_list_node_get_basic(p_syntax_decl p_decl) {
    return p_decl->b_type;
}
p_syntax_type_array syntax_decl_list_node_get_array(p_syntax_decl p_decl) {
    return p_decl->p_array;
}
p_syntax_init syntax_decl_list_node_get_init(p_syntax_decl p_decl) {
    return p_decl->p_init;
}
p_syntax_decl syntax_decl_list_node_get_entry(p_list_head p_node) {
    return list_entry(p_node, syntax_decl, node);
}
p_list_head syntax_decl_list_node_get_node(p_syntax_decl p_decl) {
    return &p_decl->node;
}
void syntax_decl_list_node_set_basic(p_syntax_decl p_decl, basic_type b_type) {
    p_decl->b_type = b_type;
}
p_syntax_decl syntax_decl_arr(p_syntax_decl p_decl, p_ast_exp p_exp) {
    size_t size = 0;
    if (p_exp) { // TODO
        assert(p_exp->p_type->basic == type_int);
        assert(p_exp->intconst > 0);
        size = p_exp->intconst;
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
