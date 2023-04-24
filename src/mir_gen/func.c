#include <mir_gen/func.h>
#include <mir_gen.h>

p_mir_func mir_func_table_gen(size_t cnt){
    p_mir_func p_func = malloc(sizeof(*p_func) * cnt);
    for (size_t i = 0; i < cnt; ++i) {
        p_func[i] = (mir_func){
            .p_basic_block = NULL,
            .temp_sym_head = list_head_init(&(p_func + i)->temp_sym_head),
        };
    }
    return p_func;
}

void mir_func_temp_sym_add(p_mir_func p_func, p_mir_temp_sym p_temp_sym)
{
    list_add_prev(&p_temp_sym->node, &p_func->temp_sym_head);
}

void mir_func_set_block_id(p_mir_func p_func)
{
    p_mir_basic_block p_basic_block = p_func->p_basic_block;
    p_basic_block->block_id = 0;
    while(p_basic_block->p_next){
        p_basic_block->p_next->block_id = p_basic_block->block_id + 1;
        p_basic_block = p_basic_block->p_next;
    }
}

void mir_func_set_temp_id(p_mir_func p_func)
{
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_func->temp_sym_head){
        p_mir_temp_sym p_temp_sym = list_entry(p_node, mir_temp_sym, node);
        p_temp_sym->id = id ++;
    }
}

void mir_func_table_drop(p_mir_func p_func, size_t cnt)
{
    for (size_t i = 0; i < cnt; ++i) {
        while ((p_func + i)->p_basic_block) {
            p_mir_basic_block p_del = (p_func + i)->p_basic_block;
            (p_func + i)->p_basic_block = p_del->p_next;
            mir_basic_block_drop(p_del);
        }
        while(!list_head_alone(&(p_func + i)->temp_sym_head))
        {
            p_mir_temp_sym p_temp_sym = list_entry((p_func + i)->temp_sym_head.p_next, mir_temp_sym, node);
            list_del(&p_temp_sym->node);
            free(p_temp_sym);
        }
    }
    free(p_func);
}