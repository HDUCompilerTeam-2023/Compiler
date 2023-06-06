#include <frontend/symbol_table.h>

#include <hir_gen.h>

#include <program/gen.h>
#include <symbol_gen.h>

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
p_symbol_table symbol_table_gen() {
    p_symbol_table p_table = malloc(sizeof(*p_table));
    *p_table = (typeof(*p_table)) {
        .p_top_table = NULL,
        .hash = init_hash(),
        .string_hash = init_hash(),
        .level = 0,
        .next_id = 0,
        .p_program = program_gen(),
        .p_func = NULL,
    };
    return p_table;
}
void symbol_table_drop(p_symbol_table p_table) {
    assert(p_table->p_top_table == NULL);
    assert(p_table->level == 0);
    for (size_t i = 0; i < hash_MOD; ++i) {
        assert(hlist_head_empty(p_table->hash + i));
    }

    free(p_table->string_hash);
    free(p_table->hash);
    free(p_table);
}

void symbol_table_zone_push(p_symbol_table p_table) {
    p_symbol_zone pst = malloc(sizeof(*pst));
    *pst = (typeof(*pst)) {
        .p_item = NULL,
        .p_prev = p_table->p_top_table,
    };
    p_table->p_top_table = pst;
    ++p_table->level;
}

void symbol_table_zone_pop(p_symbol_table p_table) {
    p_symbol_zone del_table = p_table->p_top_table;
    p_table->p_top_table = del_table->p_prev;
    --p_table->level;

    p_symbol_item p_item = del_table->p_item;
    while (p_item) {
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

static inline p_symbol_name symbol_table_get_name(p_symbol_table p_table, const char *name) {
    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = p_table->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) {
        p_name = symbol_add_name(p_head, hash_tag, name);
    }
    else {
        assert(p_name->p_item->level < p_table->level);
    }

    return p_name;
}

p_symbol_item symbol_table_var_add(p_symbol_table p_table, p_symbol_var p_var) {
    assert(p_table->level > 0);

    p_symbol_name p_name = symbol_table_get_name(p_table, p_var->name);

    p_symbol_item p_item = malloc(sizeof(*p_item));
    *p_item = (symbol_item) {
        .p_name = p_name,
        .p_prev = p_name->p_item,
        .level = p_table->level,
        .p_next = p_table->p_top_table->p_item,
        .is_func = false,
        .p_var = p_var,
    };
    p_table->p_top_table->p_item = p_item;
    p_name->p_item = p_item;

    return p_item;
}

p_symbol_item symbol_table_func_add(p_symbol_table p_table, p_symbol_func p_func) {
    assert(p_table->level > 0);

    assert(p_table->level == 1);
    p_symbol_name p_name = symbol_table_get_name(p_table, p_func->name);

    p_symbol_item p_item = malloc(sizeof(*p_item));
    *p_item = (symbol_item) {
        .p_name = p_name,
        .p_prev = p_name->p_item,
        .level = p_table->level,
        .p_next = p_table->p_top_table->p_item,
        .is_func = true,
        .p_func = p_func,
    };
    p_table->p_top_table->p_item = p_item;
    p_name->p_item = p_item;

    program_add_function(p_table->p_program, p_func);
    return p_item;
}

p_symbol_var symbol_table_var_find(p_symbol_table p_table, const char *name) {
    assert(p_table->level > 0);

    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = p_table->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name)
        return NULL;

    assert(!p_name->p_item->is_func);
    return p_name->p_item->p_var;
}
p_symbol_func symbol_table_func_find(p_symbol_table p_table, const char *name) {
    assert(p_table->level > 0);

    size_t hash_tag = symbol_str_tag(name);
    p_hlist_head p_head = p_table->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name)
        return NULL;

    assert(p_name->p_item->is_func);
    return p_name->p_item->p_func;
}

p_symbol_str symbol_table_str_get(p_symbol_table p_table, const char *string) {
    size_t hash_tag = symbol_str_tag(string);
    p_hlist_head p_head = p_table->string_hash + (hash_tag % hash_MOD);

    p_hlist_node p_node;
    hlist_for_each(p_node, p_head) {
        p_symbol_str p_str = hlist_entry(p_node, symbol_str, h_node);
        if (!strcmp(p_str->string, string)) return p_str;
    }

    p_symbol_str p_str = symbol_str_gen(string);
    hlist_node_add(p_head, &p_str->h_node);
    program_add_str(p_table->p_program, p_str);
    return p_str;
}
