#ifndef __HIR2MIR_INFO_GEN__
#define __HIR2MIR_INFO_GEN__

#include <hir2mir/info.h>

struct hir2mir_info{
    size_t id;
    p_mir_basic_block p_current_basic_block;
    p_mir_operand_list p_operand_list;
    p_mir_basic_block_list p_basic_block_list;
};

p_hir2mir_info hir2mir_info_gen(void);
p_hir2mir_info hir2mir_info_set_current_block(p_mir_basic_block p_block);
p_mir_basic_block hir2mir_info_get_current_block(p_hir2mir_info p_info);

void hir2mir_info_drop(p_hir2mir_info p_info);
#include <mir_gen.h>
#endif