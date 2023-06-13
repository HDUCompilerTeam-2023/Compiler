#include <frontend/syntax/init/def.h>

#include <ast_gen/exp.h>
#include <symbol_gen.h>

p_syntax_init syntax_init_list_gen(void) {
    p_syntax_init p_init = malloc(sizeof(*p_init));
    *p_init = (syntax_init) {
        .syntax_const = true,
        .is_exp = false,
        .list = list_head_init(&p_init->list),
        .node = list_head_init(&p_init->node),
    };
    return p_init;
}
p_syntax_init syntax_init_exp_gen(p_ast_exp p_exp) {
    p_syntax_init p_init = malloc(sizeof(*p_init));
    *p_init = (syntax_init) {
        .syntax_const = (p_exp->kind == ast_exp_num),
        .is_exp = true,
        .p_exp = p_exp,
        .node = list_head_init(&p_init->node),
    };
    return p_init;
}
p_syntax_init syntax_init_list_add(p_syntax_init p_list, p_syntax_init p_init) {
    assert(!p_list->is_exp);
    if (!p_init->syntax_const) p_list->syntax_const = false;
    assert(list_add_prev(&p_init->node, &p_list->list));
    return p_list;
}


static inline void syntax_init_list_trans(p_symbol_type p_type, basic_type basic, p_syntax_init p_srcs, p_ast_exp *memory) {
    assert(!p_srcs->is_exp);

    size_t offset = 0;
    while (!list_head_alone(&p_srcs->list)) {
        p_syntax_init p_init = list_entry(p_srcs->list.p_next, syntax_init, node);
        list_del(&p_init->node);

        if (p_init->is_exp) {
            assert(offset < symbol_type_get_size(p_type));
            p_init->p_exp = ast_exp_ptr_to_val_check_basic(p_init->p_exp);
            assert(basic == p_init->p_exp->p_type->basic);
            memory[offset++] = p_init->p_exp;
        }
        else {
            assert(!list_head_alone(&p_type->array));
            p_symbol_type_array p_pop = symbol_type_pop_array(p_type);
            assert(offset % symbol_type_get_size(p_type) == 0);
            syntax_init_list_trans(p_type, basic, p_init, memory + offset);
            offset += symbol_type_get_size(p_type);
            symbol_type_push_array(p_type, p_pop);
        }
        free(p_init);
    }
    for (; offset < symbol_type_get_size(p_type); ++offset) {
        if (basic == type_int) memory[offset] = ast_exp_int_gen(0);
        else
            memory[offset] = ast_exp_float_gen(0);
    }
}
p_syntax_init_mem syntax_init_mem_gen(p_syntax_init p_init, p_symbol_type p_type) {
    if (!p_init)
        return NULL;

    size_t size = symbol_type_get_size(p_type);
    p_syntax_init_mem p_init_mem = malloc(sizeof(*p_init_mem));
    *p_init_mem = (syntax_init_mem) {
        .size = size,
        .memory = malloc(sizeof(**p_init_mem->memory) * size)
    };
    memset(p_init_mem->memory, 0, sizeof(**p_init_mem->memory) * size);

    if (p_init->is_exp) {
        assert(list_head_alone(&p_type->array));
        p_init->p_exp = ast_exp_ptr_to_val_check_basic(p_init->p_exp);
        p_init_mem->memory[0] = p_init->p_exp;
    }
    else {
        assert(!list_head_alone(&p_type->array));
        syntax_init_list_trans(p_type, p_type->basic, p_init, p_init_mem->memory);
    }
    free(p_init);
    return p_init_mem;
}
void syntax_init_mem_drop(p_syntax_init_mem p_init) {
    if (!p_init)
        return;
    for (size_t i = 0; i < p_init->size; ++i) {
        if (p_init->memory[i]) ast_exp_drop(p_init->memory[i]);
    }
    free(p_init->memory);
    free(p_init);
}
