#include <frontend/syntax/type/def.h>

p_syntax_type_array syntax_type_array_gen(uint64_t size) {
    p_syntax_type_array p_array = malloc(sizeof(*p_array));
    *p_array = (syntax_type_array) {
        .size = size,
        .p_prev = NULL,
    };
    return p_array;
}
