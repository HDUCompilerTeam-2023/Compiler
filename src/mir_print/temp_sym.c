#include <mir_print.h>
#include <mir/temp_sym.h>

#include <stdio.h>
void mir_temp_sym_print(p_mir_temp_sym p_temp_sym)
{
    assert(p_temp_sym);
    if(p_temp_sym->is_pointer)
    {
        printf("[ 0 X ");
        mir_basic_type_print(p_temp_sym->b_type);
        printf("]* %%t%ld ", p_temp_sym->id);    
    }
    else {
        mir_basic_type_print(p_temp_sym->b_type);
        printf(" %%t%ld ", p_temp_sym->id);
    }
}