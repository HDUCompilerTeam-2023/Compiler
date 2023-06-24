#include <frontend/syntax/decl_head/def.h>

bool syntax_decl_head_get_is_const(p_syntax_decl_head p_list) {
    return p_list->is_const;
}
basic_type syntax_decl_head_get_basic(p_syntax_decl_head p_list) {
    return p_list->type;
}
