#ifndef __HIR_BLOCK__
#define __HIR_BLOCK__

#include <hir.h>

struct hir_block {
    list_head stmt;
};

#endif