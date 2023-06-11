#include <frontend/syntax/decl_list/def.h>

p_syntax_decl_list syntax_decl_list_gen(void) {
    p_syntax_decl_list p_decl_list = malloc(sizeof(*p_decl_list));
    *p_decl_list = (syntax_decl_list) {
        .decl = list_head_init(&p_decl_list->decl),
        .is_const = false,
        .type = type_void,
    };
    return p_decl_list;
}
void syntax_decl_list_drop(p_syntax_decl_list p_decl_list) {
    free(p_decl_list);
}
