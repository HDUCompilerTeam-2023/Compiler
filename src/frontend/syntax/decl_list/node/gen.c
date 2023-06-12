#include <frontend/syntax/decl_list/node/def.h>

#include <ast_gen/exp.h>
#include <frontend/syntax/type/gen.h>

p_syntax_decl syntax_decl_gen(char *name) {
    p_syntax_decl p_decl = malloc(sizeof(*p_decl));
    *p_decl = (syntax_decl) {
        .name = name,
        .p_array = NULL,
        .p_init = NULL,
        .node = list_head_init(&p_decl->node),
    };
    return p_decl;
}
void syntax_decl_drop(p_syntax_decl p_decl) {
    free(p_decl->name);
    free(p_decl);
}
