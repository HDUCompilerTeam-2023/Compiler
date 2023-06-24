#include <frontend/syntax/decl_head/def.h>

p_syntax_decl_head syntax_decl_head_gen(basic_type b_type, bool is_const) {
    p_syntax_decl_head p_decl_list = malloc(sizeof(*p_decl_list));
    *p_decl_list = (syntax_decl_head) {
        .is_const = is_const,
        .type = b_type,
    };
    return p_decl_list;
}
void syntax_decl_head_drop(p_syntax_decl_head p_decl_list) {
    free(p_decl_list);
}
