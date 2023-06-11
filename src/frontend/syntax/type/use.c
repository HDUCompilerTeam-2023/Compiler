#include <frontend/syntax/type/def.h>

#include <symbol_gen/type.h>

p_symbol_type syntax_type_trans(p_syntax_type_array p_array, basic_type b_type) {
    p_symbol_type p_type = symbol_type_var_gen(b_type);
    while (p_array) {
        p_syntax_type_array p_del = p_array;
        p_array = p_array->p_prev;
        if (p_del->size) {
            p_symbol_type_array p_add = symbol_type_array_gen(p_del->size);
            symbol_type_push_array(p_type, p_add);
        }
        else {
            symbol_type_push_ptr(p_type);
        }
        free(p_del);
    }
    return p_type;
}
p_syntax_type_array syntax_type_add_array(p_syntax_type_array p_array, p_syntax_type_array p_new_head) {
    p_new_head->p_prev = p_array;
    return p_new_head;
}
