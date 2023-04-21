#include <symbol_gen/str.h>

p_symbol_str symbol_str_gen(const char *string) {
    p_symbol_str p_str = malloc(sizeof(*p_str));
    *p_str = (symbol_str) {
        .h_node = hlist_init_node,
        .node = list_head_init(&p_str->node),
        .string = malloc(sizeof(char) * (strlen(string) + 1)),
        .length = strlen(string),
    };
    strcpy(p_str->string, string);
    return p_str;
}
void symbol_str_drop(p_symbol_str p_str) {
    list_del(&p_str->node);
    free(p_str->string);
    free(p_str);
}