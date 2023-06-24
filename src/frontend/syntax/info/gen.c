#include <frontend/syntax/info/def.h>

#include <frontend/syntax/symbol_table/gen.h>
#include <program/gen.h>

p_syntax_info syntax_info_gen(void) {
    p_syntax_info p_info = malloc(sizeof(*p_info));
    *p_info = (syntax_info) {
        .p_table = symbol_table_gen(),
        .p_program = program_gen(),
        .p_func = NULL,
        .p_block = NULL,
    };
    return p_info;
}
void syntax_info_drop(p_syntax_info p_info) {
    symbol_table_drop(p_info->p_table);
    assert(!p_info->p_func);
    free(p_info);
}