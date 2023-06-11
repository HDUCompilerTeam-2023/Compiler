#ifndef __FRONTEN_SYNTAX__
#define __FRONTEN_SYNTAX__

typedef void *yyscan_t;

#include <frontend/syntax/info/use.h>

#include <frontend/syntax/init/use.h>

#include <frontend/syntax/decl_list/use.h>
#include <frontend/syntax/decl_list/node/use.h>

#include <frontend/syntax/type/use.h>

#include <hir.h>

p_hir_exp syntax_val_offset(p_hir_exp p_val, p_hir_exp p_offset);

#endif
