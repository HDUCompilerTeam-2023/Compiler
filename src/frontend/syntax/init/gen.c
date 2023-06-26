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

static inline p_list_head syntax_init_list_regular(p_list_head p_list, p_list_head p_node, p_symbol_type p_type) {
    if (!p_node) {
        p_node = p_list->p_next;
    }

    assert(!list_head_alone(&p_type->array));
    p_symbol_type_array p_pop = symbol_type_pop_array(p_type);
    size_t cnt = symbol_type_array_get_size(p_pop);
    bool is_basic = list_head_alone(&p_type->array);

    for (size_t i = 0; i < cnt; ++i) {
        if (p_node == p_list) break;
        p_syntax_init p_init = list_entry(p_node, syntax_init, node);

        if (is_basic) {
            assert(p_init->is_exp);
            p_ast_exp p_init_exp = ast_exp_ptr_to_val_check_basic(p_init->p_exp);
            p_init->p_exp = ast_exp_cov_gen(p_init_exp, p_type->basic);
            p_node = p_node->p_next;
            continue;
        }

        if (!p_init->is_exp) {
            syntax_init_list_regular(&p_init->list, NULL, p_type);
            p_node = p_node->p_next;
            continue;
        }

        p_syntax_init p_ins = syntax_init_list_gen();
        list_add_prev(&p_ins->node, p_node);
        p_list_head p_end = syntax_init_list_regular(p_list, p_node, p_type);
        p_list_head p_next;
        for (p_node = p_ins->node.p_next, p_next = p_node->p_next; p_node != p_end; p_node = p_next, p_next = p_node->p_next) {
            list_del(p_node);
            list_add_prev(p_node, &p_ins->list);
        }
    }

    symbol_type_push_array(p_type, p_pop);
    return p_node;
}

p_syntax_init syntax_init_regular(p_syntax_init p_init, p_symbol_type p_type) {
    if (!p_init)
        return NULL;

    if (!p_init->is_exp) {
        syntax_init_list_regular(&p_init->list, NULL, p_type);
    }
    else {
        assert(list_head_alone(&p_type->array));
        p_ast_exp p_init_exp = ast_exp_ptr_to_val_check_basic(p_init->p_exp);
        p_init->p_exp = ast_exp_cov_gen(p_init_exp, p_type->basic);
    }

    return p_init;
}
void syntax_init_drop(p_syntax_init p_init) {
    if (!p_init)
        return;

    if (p_init->is_exp) {
        // ast_exp_drop(p_init->p_exp);
        free(p_init);
        return;
    }

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_init->list) {
        p_syntax_init p_del = list_entry(p_node, syntax_init, node);
        list_del(p_node);
        syntax_init_drop(p_del);
    }

    free(p_init);
}
