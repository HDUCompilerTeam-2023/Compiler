#include <frontend/syntax_gen.h>

#include <frontend/log.h>

#include <symbol/sym.h>

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

p_hir_func syntax_func_head(p_hir_program p_program, basic_type type, char *name, p_syntax_param_list p_param_list) {
    p_symbol_type p_type = symbol_type_func_gen(false);
    p_type->basic = type;

    p_symbol_type p_param = p_type;
    p_list_head p_node;
    list_for_each(p_node, &p_param_list->param_decl) {
        p_syntax_param_decl p_decl = list_entry(p_node, syntax_param_decl, node);
        p_symbol_type p_param_type = symbol_type_param_gen(p_decl->p_type);
        p_param->p_params = p_param_type;
        p_param = p_param->p_params;
    }

    p_symbol_item p_func_item = hir_symbol_item_add(p_program, symbol_func_gen(name, p_type), true);
    p_hir_func p_func = hir_func_gen(p_func_item, NULL);
    free(name);

    hir_symbol_zone_push(p_program);
    while (!list_head_alone(&p_param_list->param_decl)) {
        p_syntax_param_decl p_decl = list_entry(p_param_list->param_decl.p_next, syntax_param_decl, node);
        list_del(&p_decl->node);

        p_symbol_sym p_sym = symbol_var_gen(p_decl->name, p_decl->p_type, false, false, NULL);
        hir_symbol_item_add(p_program, p_sym, false);
        free(p_decl->name);
        free(p_decl);
    }
    free(p_param_list);
    return p_func;
}
p_hir_func syntax_func_end(p_hir_program p_program, p_hir_func p_func, p_hir_block p_block) {
    hir_symbol_zone_pop(p_program);
    p_func->p_block = p_block;
    return p_func;
}

typedef struct syntax_init_mem syntax_init_mem, *p_syntax_init_mem;
struct syntax_init_mem {
    size_t size;
    p_hir_exp *memory;
};
p_syntax_init_mem syntax_init_mem_gen(size_t size) {
    p_syntax_init_mem p_init = malloc(sizeof(*p_init));
    *p_init = (syntax_init_mem) {
        .size = size,
        .memory = malloc(sizeof(**p_init->memory) * size)
    };
    memset(p_init->memory, 0, sizeof(**p_init->memory) * size);
    return p_init;
}
p_syntax_init_mem syntax_init_mem_add(p_syntax_init_mem p_init, size_t offset, p_hir_exp p_exp) {
    assert(offset < p_init->size);
    p_init->memory[offset] = p_exp;
    return p_init;
}
void syntax_init_mem_drop(p_syntax_init_mem p_init) {
    if(!p_init)
        return;
    for (size_t i = 0; i < p_init->size; ++i) {
        if (p_init->memory[i]) hir_exp_drop(p_init->memory[i]);
    }
    free(p_init->memory);
    free(p_init);
}

static inline void syntax_init_list_trans(p_symbol_type p_type, basic_type basic, p_syntax_init p_srcs, p_hir_exp *memory) {
    assert(!p_srcs->is_exp);

    size_t offset = 0;
    while (!list_head_alone(&p_srcs->list)) {
        p_syntax_init p_init = list_entry(p_srcs->list.p_next, syntax_init, node);
        list_del(&p_init->node);

        if (p_init->is_exp) {
            assert(offset < p_type->size);
            assert(basic == hir_exp_get_basic(p_init->p_exp));
            memory[offset++] = p_init->p_exp;
        }
        else {
            assert(p_type->kind == type_arrary);
            assert(offset % p_type->p_item->size == 0);
            syntax_init_list_trans(p_type->p_item, basic, p_init, memory + offset);
            offset += p_type->p_item->size;
        }
        free(p_init);
    }
    for (; offset < p_type->size; ++offset) {
        if (basic == type_int) memory[offset] = hir_exp_int_gen(0);
        else memory[offset] = hir_exp_float_gen(0);
    }
}
static inline p_syntax_init_mem syntax_init_trans(p_syntax_decl p_decl) {
    if (!p_decl->p_init)
        return NULL;

    p_syntax_init_mem p_init = syntax_init_mem_gen(p_decl->p_type->size);
    if (p_decl->p_init->is_exp) {
        assert(p_decl->p_type->kind == type_var);
        assert(p_decl->p_type->basic == hir_exp_get_basic(p_decl->p_init->p_exp));
        p_init->memory[0] = p_decl->p_init->p_exp;
    }
    else {
        assert(p_decl->p_type->kind == type_arrary);
        syntax_init_list_trans(p_decl->p_type, p_decl->p_tail->basic, p_decl->p_init, p_init->memory);
    }
    free(p_decl->p_init);
    return p_init;
}
p_hir_block syntax_local_vardecl(p_hir_program p_program, p_hir_block p_block, p_syntax_decl_list p_decl_list) {
    while (!list_head_alone(&p_decl_list->decl)) {
        p_syntax_decl p_decl = list_entry(p_decl_list->decl.p_next, syntax_decl, node);
        list_del(&p_decl->node);

        syntax_decl_type_add(p_decl, symbol_type_var_gen(p_decl_list->type));

        p_syntax_init_mem p_h_init = syntax_init_trans(p_decl);
        if (p_decl_list->is_const) {
            p_symbol_init p_init = symbol_init_gen(p_h_init->size, p_decl_list->type);
            for(size_t i = 0; i < p_h_init->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = syntax_const_check(p_h_init->memory[i])->intconst;
                }
                else {
                    init_val.f = syntax_const_check(p_h_init->memory[i])->floatconst;
                }
                symbol_init_add(p_init, i, init_val);
            }
            p_symbol_sym p_sym = symbol_var_gen(p_decl->name, p_decl->p_type, p_decl_list->is_const, p_init != NULL, p_init);
            hir_symbol_item_add(p_program, p_sym, false);
        }
        else {
            p_symbol_sym p_sym = symbol_var_gen(p_decl->name, p_decl->p_type, p_decl_list->is_const, false, NULL);
            hir_symbol_item_add(p_program, p_sym, false);
            if (p_h_init) {
                p_symbol_type p_var_type = p_sym->p_type;
                while(p_var_type->kind == type_arrary) {
                    p_var_type = p_var_type->p_item;
                }
                if (p_sym->p_type->kind == type_var) {
                    p_hir_exp p_lval = hir_exp_val_gen(p_sym);
                    p_hir_exp p_rval = p_h_init->memory[0];
                    p_h_init->memory[0] = NULL;
                    p_hir_exp p_assign = hir_exp_assign_gen(p_lval, p_rval);
                    hir_block_add(p_block, hir_stmt_exp_gen(p_assign));
                }
                else {
                    for (size_t i = 0; i < p_h_init->size; ++i) {
                        p_hir_exp p_lval = hir_exp_val_gen(p_sym);
                        p_lval->p_type = p_var_type;
                        p_lval->p_offset = hir_exp_int_gen(i);
                        p_hir_exp p_rval = p_h_init->memory[i];
                        p_h_init->memory[i] = NULL;
                        p_hir_exp p_assign = hir_exp_assign_gen(p_lval, p_rval);
                        hir_block_add(p_block, hir_stmt_exp_gen(p_assign));
                    }
                }
            }
        }
        syntax_init_mem_drop(p_h_init);
        free(p_decl->name);

        free(p_decl);
    }
    free(p_decl_list);
    return p_block;
}
void syntax_global_vardecl(p_hir_program p_program, p_syntax_decl_list p_decl_list) {
    while (!list_head_alone(&p_decl_list->decl)) {
        p_syntax_decl p_decl = list_entry(p_decl_list->decl.p_next, syntax_decl, node);
        list_del(&p_decl->node);

        syntax_decl_type_add(p_decl, symbol_type_var_gen(p_decl_list->type));

        p_syntax_init_mem p_h_init = syntax_init_trans(p_decl);
        p_symbol_init p_init = symbol_init_gen(p_decl->p_type->size, p_decl_list->type);
        if (p_h_init) {
            for(size_t i = 0; i < p_init->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = syntax_const_check(p_h_init->memory[i])->intconst;
                }
                else {
                    init_val.f = syntax_const_check(p_h_init->memory[i])->floatconst;
                }
                symbol_init_add(p_init, i, init_val);
            }
        }
        else {
            for(size_t i = 0; i < p_init->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = 0;
                }
                else {
                    init_val.f = 0;
                }
                symbol_init_add(p_init, i, init_val);
            }
        }
        syntax_init_mem_drop(p_h_init);
        p_symbol_sym p_sym = symbol_var_gen(p_decl->name, p_decl->p_type, p_decl_list->is_const, p_init != NULL, p_init);
        hir_symbol_item_add(p_program, p_sym, true);

        free(p_decl->name);
        free(p_decl);
    }
    free(p_decl_list);
}

void syntax_rtlib_decl(p_hir_program p_program, basic_type type, char *name, p_symbol_type p_param1, p_symbol_type p_param2, bool is_va) {
    p_symbol_type p_type = symbol_type_func_gen(is_va);
    p_type->basic = type;

    if (p_param1) {
        p_type->p_params = symbol_type_param_gen(p_param1);
        if (p_param2) {
            p_type->p_params->p_params = symbol_type_param_gen(p_param2);
        }
    }

    p_symbol_sym p_sym = symbol_func_gen(name, p_type);
    p_symbol_item p_item = hir_symbol_item_add(p_program, p_sym, true);
    list_add_prev(&hir_func_gen(p_item, NULL)->node, &p_program->func);

    hir_symbol_zone_push(p_program);
    if (p_type->p_params) {
        p_symbol_sym p_sym = symbol_var_gen("arg1", p_type->p_params->p_item, false, false, NULL);
        hir_symbol_item_add(p_program, p_sym, false);
        if (p_type->p_params->p_params) {
            p_sym = symbol_var_gen("arg2", p_type->p_params->p_params->p_item, false, false, NULL);
            hir_symbol_item_add(p_program, p_sym, false);
        }
    }
    hir_symbol_zone_pop(p_program);
}

p_hir_exp syntax_const_check(p_hir_exp p_exp) {
    assert(p_exp->kind == hir_exp_num);
    return p_exp;
}
