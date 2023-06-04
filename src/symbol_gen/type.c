#include <symbol_gen/type.h>

p_symbol_type symbol_type_var_gen(basic_type basic) {
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type) {
        .ref_level = 0,
        .array = list_init_head(&p_type->array),
        .basic = basic,
        .size = 1,
    };
    return p_type;
}
p_symbol_type_array symbol_type_array_gen(size_t size) {
    p_symbol_type_array p_array = malloc(sizeof(*p_array));
    *p_array = (symbol_type_array) {
        .size = size,
        .node = list_init_head(&p_array->node),
    };
    return p_array;
}

void symbol_type_drop(p_symbol_type p_type) {
    assert(p_type);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_type->array) {
        p_symbol_type_array p_array = list_entry(p_node, symbol_type_array, node);
        list_del(&p_array->node);
        symbol_type_array_drop(p_array);
    }
    free(p_type);
}
void symbol_type_array_drop(p_symbol_type_array p_array) {
    free(p_array);
}

void symbol_type_push_array(p_symbol_type p_type, p_symbol_type_array p_array) {
    list_add_next(&p_array->node, &p_type->array);
    p_type->size *= p_array->size;
}
p_symbol_type_array symbol_type_top_array(p_symbol_type p_type) {
    p_symbol_type_array p_array = list_entry(p_type->array.p_next, symbol_type_array, node);
    return p_array;
}
p_symbol_type_array symbol_type_pop_array(p_symbol_type p_type) {
    assert(p_type->ref_level == 0);
    p_symbol_type_array p_array = list_entry(p_type->array.p_next, symbol_type_array, node);
    list_del(&p_array->node);
    p_type->size /= p_array->size;
    return p_array;
}

void symbol_type_push_ptr(p_symbol_type p_type) {
    ++(p_type->ref_level);
}
void symbol_type_pop_ptr(p_symbol_type p_type) {
    --(p_type->ref_level);
}

p_symbol_type symbol_type_copy(p_symbol_type p_type) {
    p_symbol_type p_copy = symbol_type_var_gen(p_type->basic);
    p_list_head p_node;
    list_for_each_tail(p_node, &p_type->array) {
        p_symbol_type_array p_array = list_entry(p_node, symbol_type_array, node);
        p_symbol_type_array p_array_copy = symbol_type_array_copy(p_array);
        symbol_type_push_array(p_copy, p_array_copy);
    }
    p_copy->ref_level = p_type->ref_level;

    return p_copy;
}
p_symbol_type_array symbol_type_array_copy(p_symbol_type_array p_array) {
    p_symbol_type_array p_copy = symbol_type_array_gen(p_array->size);
    return p_copy;
}

uint64_t symbol_type_get_size(p_symbol_type p_type) {
    return p_type->size;
}
uint64_t symbol_type_array_get_size(p_symbol_type_array p_array) {
    assert(p_array);
    return p_array->size;
}
