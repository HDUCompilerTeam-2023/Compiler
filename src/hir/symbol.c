#include <hir/symbol.h>

#include <hlist.h>

#define hash_P (65537)
#define hash_MOD (109)

struct symbol_item {
    p_symbol_name p_name;
    p_symbol_item p_prev;

    uint16_t level;
    p_symbol_item p_next;

    p_symbol_info p_info;
};

struct symbol_name {
    char * name;
    p_symbol_item p_item;

    size_t hash_tag;
    hlist_node node;
};

struct symbol_table {
    p_symbol_item p_item;

    p_symbol_table p_prev;
};

struct symbol_store {
    hlist_hash hash;

    p_symbol_info p_info;

    uint16_t level;
    p_symbol_table p_top_table;
};

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
    *pss = (typeof(*pss)) {
        .p_info = NULL,
        .p_top_table = NULL,
        .hash = init_hash(),
        .level = 0,
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
        p_item->p_info->p_next = pss->p_info;
        pss->p_info = p_item->p_info;

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

    while (pss->p_info) {
        p_symbol_info del = pss->p_info;
        pss->p_info = del->p_next;
        free(del);
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

bool symbol_add(p_symbol_store pss, const char *name) {
    assert(pss->level > 0);

    size_t hash_tag = symbol_str_tag("name");
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) {
        p_name = symbol_add_name(p_head, hash_tag, name);
    }
    else if (p_name->p_item->level == pss->level) return false;
    else assert(p_name->p_item->level < pss->level);

    p_symbol_info p_info = malloc(sizeof(*p_info));
    *p_info = (symbol_info) {
        .p_next = NULL,
    };

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
    return true;
}

p_symbol_info symbol_find(p_symbol_store pss, const char *name) {
    assert(pss->level > 0);

    size_t hash_tag = symbol_str_tag("name");
    p_hlist_head p_head = pss->hash + (hash_tag % hash_MOD);

    p_symbol_name p_name = symbol_find_name(p_head, name);
    if (!p_name) return NULL;

    return p_name->p_item->p_info;
}