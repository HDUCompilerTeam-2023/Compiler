#include <frontend/syntax/init/def.h>

#include <hir_gen/exp.h>
#include <symbol_gen/type.h>
#include <frontend/syntax/init/gen.h>

p_hir_exp syntax_init_mem_get_exp(p_syntax_init_mem p_init_mem, size_t offset) {
    assert(offset < p_init_mem->size);
    return p_init_mem->memory[offset];
}
void syntax_init_mem_clear_exp(p_syntax_init_mem p_init_mem, size_t offset) {
    assert(offset < p_init_mem->size);
    p_init_mem->memory[offset] = NULL;
}
