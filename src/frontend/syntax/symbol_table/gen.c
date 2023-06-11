#include <frontend/syntax/symbol_table/def.h>

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

