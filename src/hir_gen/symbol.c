#include <hir_gen/symbol.h>

#include <hir_gen.h>

#define hash_P (65537)
#define hash_MOD (109)

p_symbol_init symbol_init_gen(size_t size) {
    p_symbol_init p_init = malloc(sizeof(*p_init));
    *p_init = (symbol_init) {
        .size = size,
        .memory = malloc(sizeof(**p_init->memory) * size)
    };
    memset(p_init->memory, 0, sizeof(**p_init->memory) * size);
    return p_init;
}
p_symbol_init symbol_init_add(p_symbol_init p_init, size_t offset, p_hir_exp p_exp) {
    assert(offset < p_init->size);
    p_init->memory[offset] = p_exp;
    return p_init;
}
void symbol_init_drop(p_symbol_init p_init) {
    if(!p_init)
        return;
    for(size_t i = 0; i < p_init->size; ++i) {
        if (p_init->memory[i]) {
            hir_exp_drop(p_init->memory[i]);
        }
    }
    free(p_init->memory);
    free(p_init);
}

static inline hlist_hash init_hash() {
    hlist_hash hash = malloc(sizeof(*hash) * hash_MOD);
    for (size_t i = 0; i < hash_MOD; ++i)
        hlist_head_init(hash + i);
    return hash;
}

static inline size_t symbol_str_tag(const char *name) {
    size_t hash = 0;
    const char *ptr = name;
    while (*ptr) {
        hash = hash * hash_P + *(ptr++);
    }
    return hash;
}

p_symbol_store symbol_store_initial() {
    p_symbol_store pss = malloc(sizeof(*pss));
    *pss = (typeof(*pss)){
        .global = list_head_init(&pss->global),
        .def_function = list_head_init(&pss->def_function),
        .ndef_function = list_head_init(&pss->ndef_function),
        .p_top_table = NULL,
        .hash = init_hash(),
        .level = 0,
        .next_id = 0,
    };
    return pss;
}

void symbol_push_zone(p_symbol_store pss) {
    p_symbol_table pst = malloc(sizeof(*pst));
    *pst = (typeof(*pst)) {
        .p_item = NULL,
        .p_prev = pss->p_top_table,
    };
    pss->p_top_table = pst;
    ++pss->level;
}

void symbol_pop_zone(p_symbol_store pss) {
    p_symbol_table del_table = pss->p_top_table;
    pss->p_top_table = del_table->p_prev;
    --pss->level;

    p_symbol_item p_item = del_table->p_item;
    while(p_item) {
        p_item->p_name->p_item = p_item->p_prev;
        if (!p_item->p_name->p_item) {
            hlist_node_del(&p_item->p_name->node);
            free(p_item->p_name->name);
            free(p_item->p_name);
        }

        p_symbol_item del_item = p_item;
        p_item = p_item->p_next;
        free(del_item);
    }

    free(del_table);
}

void symbol_store_destroy(p_symbol_store pss) {
    assert(pss->p_top_table == NULL);
    assert(pss->level == 0);
    for (size_t i = 0; i < hash_MOD; ++i) {
        assert(hlist_head_empty(pss->hash + i));
    }

    while (!list_head_alone(&pss->def_function)) {
        p_symbol_sym p_del = list_entry(pss->def_function.p_next, symbol_sym, node);
        list_del(&p_del->node);
        while (!list_head_alone(&p_del->local)) {
            p_symbol_sym p_del_l = list_entry(p_del->local.p_next, symbol_sym, node);
            list_del(&p_del_l->node);
            symbol_init_drop(p_del_l->p_init);
            symbol_type_drop(p_del_l->p_type);
            free(p_del_l->name);
            free(p_del_l);
        }
        hir_func_drop(p_del->p_func);
        symbol_type_drop(p_del->p_type);
        free(p_del->name);
        free(p_del);
    }

    while (!list_head_alone(&pss->ndef_function)) {
        p_symbol_sym p_del = list_entry(pss->ndef_function.p_next, symbol_sym, node);
        list_del(&p_del->node);
        symbol_type_drop_param(p_del->p_type);
        symbol_type_drop(p_del->p_type);
        free(p_del->name);
        free(p_del);
    }

    while (!list_head_alone(&pss->global)) {
        p_symbol_sym p_del = list_entry(pss->global.p_next, symbol_sym, node);
        list_del(&p_del->node);
        symbol_init_drop(p_del->p_init);
        symbol_type_drop(p_del->p_type);
        free(p_del->name);
        free(p_del);
    }

    free(pss->hash);
    free(pss);
}

static inline p_symbol_name symbol_find_name(p_hlist_head p_head, const char *name) {
    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_symbol_name p_name = hlist_entry(p_node, symbol_name, node);
        if (!strcmp(p_name->name, name)) return p_name;
    }
    return NULL;
}

static inline p_symbol_name symbol_add_name(p_hlist_head p_head, size_t hash_tag, const char *name) {
    p_symbol_name p_name = malloc(sizeof(*p_name));
    *p_name = (symbol_name) {
        .p_item = NULL,
        .node = hlist_init_node,
        .hash_tag = hash_tag,
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
    };
    strcpy(p_name->name, name);
    hlist_node_add(p_head, &p_name->node);
    return p_name;
}

p_symbol_sym symbol_add(p_symbol_store pss, const char *name, p_symbol_type p_type, bool is_const, bool is_def, void *p_data) {
    assert(pss->level > 0);

    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) {
        p_name = symbol_add_name(p_head, hash_tag, name);
    }
    else if (p_name->p_item->level == pss->level) return NULL;
    else assert(p_name->p_item->level < pss->level);

    p_symbol_sym p_info = malloc(sizeof(*p_info));
    *p_info = (symbol_sym) {
        .node = list_head_init(&p_info->node),
        .is_def = is_def,
        .is_const = is_const,
        .name = malloc(sizeof(char) * (strlen(name) + 1)),
        .p_type = p_type,
    };
    strcpy(p_info->name, name);
    if (p_type->kind == type_func) {
        p_info->p_func = (p_hir_func) p_data;
        p_info->local = list_head_init(&p_info->local);
    }
    else {
        p_info->p_init = (p_symbol_init) p_data;
        p_info->id = pss->next_id++;
        p_info->is_global = !pss->p_top_table->p_prev;
    }

    if (p_info->p_type->kind == type_func) {
        if (p_info->is_def) {
            list_add_prev(&p_info->node, &pss->def_function);
        }
        else {
            list_add_prev(&p_info->node, &pss->ndef_function);
        }
    }
    else {
        if (p_info->is_global) {
            list_add_prev(&p_info->node, &pss->global);
        }
        else {
            assert(!list_head_alone(&pss->def_function));
            p_symbol_sym p_func = list_entry(pss->def_function.p_prev, symbol_sym, node);
            list_add_prev(&p_info->node, &p_func->local);
        }
    }

    p_symbol_item p_item = malloc(sizeof(*p_item));
    *p_item = (symbol_item) {
        .p_name = p_name,
        .p_prev = p_name->p_item,
        .level = pss->level,
        .p_next = pss->p_top_table->p_item,
        .p_info = p_info,
    };
    pss->p_top_table->p_item = p_item;
    p_name->p_item = p_item;
    return p_info;
}

p_symbol_sym symbol_find(p_symbol_store pss, const char *name) {
    assert(pss->level > 0);

    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) return NULL;

    return p_name->p_item->p_info;
}