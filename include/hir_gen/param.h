#ifndef __HIR_GEN_PARAM__
#define __HIR_GEN_PARAM__

#include <hir/param.h>

p_hir_param_list hir_param_list_init(void);
p_hir_param_list hir_param_list_add(p_hir_param_list p_head, p_hir_exp p_exp);
void hir_param_list_drop(p_hir_param_list p_param_list);

#endif