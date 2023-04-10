#include <frontend/syntax_gen.h>

#include <frontend/log.h>

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
p_syntax_init syntax_init_list_add(p_syntax_init p_list, p_syntax_init p_init) {
    assert(!p_list->is_exp);
    if (!p_init->syntax_const) p_list->syntax_const = false;
    assert(list_add_prev(&p_init->node, &p_list->list));
    return p_list;
}
p_syntax_init syntax_init_exp_gen(p_hir_exp p_exp) {
    p_syntax_init p_init = malloc(sizeof(*p_init));
    *p_init = (syntax_init) {
        .syntax_const = (p_exp->kind == hir_exp_num),
        .is_exp = true,
        .p_exp = p_exp,
        .node = list_head_init(&p_init->node),
    };
    return p_init;
}
void syntax_init_drop(p_syntax_init p_init) {
    assert(p_init);
    if (p_init->is_exp) {
        hir_exp_drop(p_init->p_exp);
        free(p_init);
        return;
    }

    while (!list_head_alone(&p_init->list)) {
        p_syntax_init p_del = list_entry(p_init->list.p_next, syntax_init, node);
        list_del(&p_del->node);

        syntax_init_drop(p_del);
    }

    free(p_init);
}

void syntax_decl_type_add(p_syntax_decl p_decl, p_symbol_type p_tail) {
    if (p_decl->p_tail) {
        p_decl->p_tail->p_item = p_tail;
        if (p_tail->kind == type_arrary) {
            assert(p_decl->p_type->kind == type_arrary);
            p_symbol_type p_node = p_decl->p_type;
            while (p_node != p_tail) {
                p_node->size *= p_tail->size;
                p_node = p_node->p_item;
            }
        }
    }
    else p_decl->p_type = p_tail;
    p_decl->p_tail = p_tail;
}
p_syntax_decl syntax_decl_gen(char *name) {
    p_syntax_decl p_decl = malloc(sizeof(*p_decl));
    *p_decl = (syntax_decl) {
        .name = name,
        .p_type = NULL,
        .p_tail = NULL,
        .p_init = NULL,
        .node = list_head_init(&p_decl->node),
    };
    return p_decl;
}
p_syntax_decl syntax_decl_arr(p_syntax_decl p_decl, p_hir_exp p_exp) {
    size_t size = 0;
    if (p_exp) { // TODO
        assert(p_exp->basic == type_int);
        assert(p_exp->intconst > 0);
        size = p_exp->intconst;
        free(p_exp);
    }
    p_symbol_type p_arrary = symbol_type_arrary_gen(size);
    syntax_decl_type_add(p_decl, p_arrary);

    return p_decl;
}
p_syntax_decl syntax_decl_init(p_syntax_decl p_decl, p_syntax_init p_init) {
    p_decl->p_init = p_init;
    return p_decl;
}

p_syntax_decl_list syntax_decl_list_gen(void) {
    p_syntax_decl_list p_decl_list = malloc(sizeof(*p_decl_list));
    *p_decl_list = (syntax_decl_list) {
        .decl = list_head_init(&p_decl_list->decl),
        .is_const = false,
        .type = type_void,
    };
    return p_decl_list;
}
p_syntax_decl_list syntax_decl_list_add(p_syntax_decl_list p_decl_list, p_syntax_decl p_decl) {
    assert(list_add_prev(&p_decl->node, &p_decl_list->decl));
    return p_decl_list;
}
p_syntax_decl_list syntax_decl_list_set(p_syntax_decl_list p_decl_list, bool is_const, basic_type type) {
    p_decl_list->is_const = is_const;
    p_decl_list->type = type;

    return p_decl_list;
}

p_syntax_param_decl syntax_param_decl_gen(basic_type type, p_syntax_decl p_decl) {
    syntax_decl_type_add(p_decl, symbol_type_var_gen(type));

    p_syntax_param_decl p_param_decl = malloc(sizeof(*p_param_decl));
    *p_param_decl = (syntax_param_decl) {
        .name = p_decl->name,
        .p_type = p_decl->p_type,
        .node = list_head_init(&p_param_decl->node),
    };
    free(p_decl);
    return p_param_decl;
}

p_syntax_param_list syntax_param_list_gen(void) {
    p_syntax_param_list p_param_list = malloc(sizeof(*p_param_list));
    *p_param_list = (syntax_param_list) {
        .param_decl = list_head_init(&p_param_list->param_decl),
    };
    return p_param_list;
}

p_syntax_param_list syntax_param_list_add(p_syntax_param_list p_list, p_syntax_param_decl p_decl) {
    if (!p_list) p_list = syntax_param_list_gen();
    assert(list_add_prev(&p_decl->node, &p_list->param_decl));
    return p_list;
}

void syntax_func_define(p_symbol_store pss, basic_type type, char *name, p_syntax_param_list p_param_list) {
    p_symbol_type p_type = symbol_type_func_gen();
    p_type->basic = type;

    p_symbol_type p_param = p_type;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param_decl) {
        p_syntax_param_decl p_decl = list_entry(p_node, syntax_param_decl, node);
        p_symbol_type p_param_type = symbol_type_param_gen(p_decl->p_type);
        p_param->p_params = p_param_type;
        p_param = p_param->p_params;
    }

    symbol_add(pss, name, p_type, false, true, NULL);
}
void syntax_func_param(p_symbol_store pss, p_syntax_param_list p_param_list) {
    while (!list_head_alone(&p_param_list->param_decl)) {
        p_syntax_param_decl p_decl = list_entry(p_param_list->param_decl.p_next, syntax_param_decl, node);
        list_del(&p_decl->node);

        symbol_add(pss, p_decl->name, p_decl->p_type, false, false, NULL);
        free(p_decl->name);
        free(p_decl);
    }
    free(p_param_list);
}

p_hir_exp syntax_local_symbol_init(p_symbol_sym p_sym, p_syntax_init p_init) {
    syntax_init_drop(p_init);
    return NULL;
}
#include <stdio.h>
p_hir_block syntax_local_vardecl(p_symbol_store pss, p_hir_block p_block, p_syntax_decl_list p_decl_list) {
    while (!list_head_alone(&p_decl_list->decl)) {
        p_syntax_decl p_decl = list_entry(p_decl_list->decl.p_next, syntax_decl, node);
        list_del(&p_decl->node);

        syntax_decl_type_add(p_decl, symbol_type_var_gen(p_decl_list->type));
        
        p_symbol_sym p_sym = symbol_add(pss, p_decl->name, p_decl->p_type, p_decl_list->is_const, false, NULL);
        free(p_decl->name);
        if (p_decl->p_init) {
            printf("init ");
            hir_block_add(p_block,
                hir_stmt_exp_gen(
                    syntax_local_symbol_init(p_sym, p_decl->p_init)
                )
            );
            printf("\n");
        }

        free(p_decl);
    }
    free(p_decl_list);
    return p_block;
}
void syntax_global_symbol_init(p_symbol_sym p_sym, p_syntax_init p_init) {
    syntax_init_drop(p_init);
}
void syntax_global_vardecl(p_symbol_store pss, p_syntax_decl_list p_decl_list) {
    while (!list_head_alone(&p_decl_list->decl)) {
        p_syntax_decl p_decl = list_entry(p_decl_list->decl.p_next, syntax_decl, node);
        list_del(&p_decl->node);

        syntax_decl_type_add(p_decl, symbol_type_var_gen(p_decl_list->type));

        p_symbol_sym p_sym = symbol_add(pss, p_decl->name, p_decl->p_type, p_decl_list->is_const, true, NULL);
        free(p_decl->name);
        if (p_decl->p_init) {
            printf("init ");
            syntax_global_symbol_init(p_sym, p_decl->p_init);
            printf("\n");
        }
        free(p_decl);
    }
    free(p_decl_list);
}

p_hir_exp syntax_const_check(p_hir_exp p_exp) {
    assert(p_exp->kind == hir_exp_num);
    return p_exp;
}
