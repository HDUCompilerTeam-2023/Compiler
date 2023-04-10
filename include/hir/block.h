#ifndef __HIR_BLOCK__
#define __HIR_BLOCK__

#include <hir.h>

struct hir_block {
    uint64_t length;
    list_head stmt;
};

#endif