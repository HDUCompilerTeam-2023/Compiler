#include <frontend/syntax/init/def.h>

#include <ast_gen/exp.h>
#include <symbol_gen/type.h>
#include <frontend/syntax/init/gen.h>

p_ast_exp syntax_init_get_exp(p_syntax_init p_init, p_symbol_type p_type, size_t offset) {
    if (!p_init) return NULL;

    assert(offset < symbol_type_get_size(p_type));
    if (list_head_alone(&p_type->array)) {
        assert(p_init->is_exp);
        return p_init->p_exp;
    }

    p_symbol_type_array p_pop = symbol_type_pop_array(p_type);
    size_t len = symbol_type_get_size(p_type);
    size_t index = offset / len;

    assert(!p_init->is_exp);
    p_list_head p_node = p_init->list.p_next;
    for (size_t i = 0; p_node != &p_init->list && i < index; ++i) {
        p_node = p_node->p_next;
    }

    if (p_node == &p_init->list) {
        symbol_type_push_array(p_type, p_pop);
        return NULL;
    }
    p_syntax_init p_inner = list_entry(p_node, syntax_init, node);
    p_ast_exp p_ret = syntax_init_get_exp(p_inner, p_type, offset % len);

    symbol_type_push_array(p_type, p_pop);
    return p_ret;
}
