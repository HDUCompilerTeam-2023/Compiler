#include <frontend/syntax/decl_list/def.h>

bool syntax_decl_list_get_is_const(p_syntax_decl_list p_list) {
    return p_list->is_const;
}
p_list_head syntax_decl_list_get_list(p_syntax_decl_list p_list) {
    return &p_list->decl;
}
basic_type syntax_decl_list_get_basic(p_syntax_decl_list p_list) {
    return p_list->type;
}
p_syntax_decl_list syntax_decl_list_add(p_syntax_decl_list p_decl_list, p_syntax_decl p_decl) {
    assert(list_add_prev(syntax_decl_list_node_get_node(p_decl), &p_decl_list->decl));
    return p_decl_list;
}
p_syntax_decl_list syntax_decl_list_set(p_syntax_decl_list p_decl_list, bool is_const, basic_type type) {
    p_decl_list->is_const = is_const;
    p_decl_list->type = type;

    return p_decl_list;
}
