#ifndef __HIR_PARAM__
#define __HIR_PARAM__

#include <hir.h>

struct hir_param_list {
    list_head param;
};

struct hir_param {
    p_hir_exp p_exp;
    list_head node;
};

#endif