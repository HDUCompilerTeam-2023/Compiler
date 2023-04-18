#include <hir2mir/info_gen.h>

p_hir2mir_info hir2mir_info_gen(){
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info){
        .id = 1,
        .p_current_basic_block = NULL,
        .p_operand_list = mir_operand_list_gen(),
        .p_basic_block_list = mir_basic_block_list_gen(),
    };
    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info)
{
    free(p_info);
}
