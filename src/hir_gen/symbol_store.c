#include <hir_gen/symbol_store.h>

#include <hir_gen.h>

#define hash_P (65537)
#define hash_MOD (109)

static inline size_t symbol_str_tag(const char *name) {
    size_t hash = 0;
    const char *ptr = name;
    while (*ptr) {
        hash = hash * hash_P + *(ptr++);
    }
    return hash;
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

static inline hlist_hash init_hash() {
    hlist_hash hash = malloc(sizeof(*hash) * hash_MOD);
    for (size_t i = 0; i < hash_MOD; ++i)
        hlist_head_init(hash + i);
    return hash;
}
p_symbol_store symbol_store_initial() {
    p_symbol_store pss = malloc(sizeof(*pss));
    *pss = (typeof(*pss)){
        .global = list_head_init(&pss->global),
        .def_function = list_head_init(&pss->def_function),
        .ndef_function = list_head_init(&pss->ndef_function),
        .string = list_head_init(&pss->string),
        .p_top_table = NULL,
        .hash = init_hash(),
        .string_hash = init_hash(),
        .level = 0,
        .next_id = 0,
    };
    return pss;
}
void symbol_store_destroy(p_symbol_store pss) {
    assert(pss->p_top_table == NULL);
    assert(pss->level == 0);
    for (size_t i = 0; i < hash_MOD; ++i) {
        assert(hlist_head_empty(pss->hash + i));
    }

    while (!list_head_alone(&pss->def_function)) {
        p_symbol_sym p_del = list_entry(pss->def_function.p_next, symbol_sym, node);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&pss->ndef_function)) {
        p_symbol_sym p_del = list_entry(pss->ndef_function.p_next, symbol_sym, node);
        symbol_func_drop(p_del);
    }

    while (!list_head_alone(&pss->global)) {
        p_symbol_sym p_del = list_entry(pss->global.p_next, symbol_sym, node);
        symbol_var_drop(p_del);
    }

    while (!list_head_alone(&pss->string)) {
        p_symbol_str p_del = list_entry(pss->string.p_next, symbol_str, node);
        symbol_str_drop(p_del);
    }

    free(pss->string_hash);
    free(pss->hash);
    free(pss);
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

bool symbol_add(p_symbol_store pss, p_symbol_sym p_sym) {
    assert(pss->level > 0);

    if (p_sym->p_type->kind >= type_func) {
        if (p_sym->is_def) {
            list_add_prev(&p_sym->node, &pss->def_function);
        }
        else {
            list_add_prev(&p_sym->node, &pss->ndef_function);
        }
    }
    else {
        p_sym->id = pss->next_id++;
        p_sym->is_global = !pss->p_top_table->p_prev;
        if (p_sym->is_global) {
            list_add_prev(&p_sym->node, &pss->global);
        }
        else {
            assert(!list_head_alone(&pss->def_function));
            p_symbol_sym p_func = list_entry(pss->def_function.p_prev, symbol_sym, node);
            list_add_prev(&p_sym->node, &p_func->local);
        }
    }

    size_t hash_tag = symbol_str_tag(p_sym->name);
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, p_sym->name);
    if (!p_name) {
        p_name = symbol_add_name(p_head, hash_tag, p_sym->name);
    }
    else if (p_name->p_item->level == pss->level) {
        if (p_sym->p_type->kind >= type_func) {
            symbol_func_drop(p_sym);
        }
        else {
            symbol_var_drop(p_sym);
        }
        return false;
    }
    else assert(p_name->p_item->level < pss->level);

    p_symbol_item p_item = malloc(sizeof(*p_item));
    *p_item = (symbol_item) {
        .p_name = p_name,
        .p_prev = p_name->p_item,
        .level = pss->level,
        .p_next = pss->p_top_table->p_item,
        .p_info = p_sym,
    };
    pss->p_top_table->p_item = p_item;
    p_name->p_item = p_item;

    return true;
}

p_symbol_sym symbol_find(p_symbol_store pss, const char *name) {
    assert(pss->level > 0);

    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) return NULL;

    return p_name->p_item->p_info;
}

p_symbol_str symbol_str_find(p_symbol_store pss, const char *string) {
    size_t hash_tag = symbol_str_tag(string);
    p_hlist_head p_head = pss->string_hash + (hash_tag % hash_MOD);

    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_symbol_str p_str = hlist_entry(p_node, symbol_str, h_node);
        if (!strcmp(p_str->string, string)) return p_str;
    }

    p_symbol_str p_str = symbol_str_gen(string);
    hlist_node_add(p_head, &p_str->h_node);

    list_add_prev(&p_str->node, &pss->string);
    return p_str;
}
