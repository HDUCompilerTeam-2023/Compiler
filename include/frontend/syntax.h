#ifndef __FRONTEN_SYNTAX__
#define __FRONTEN_SYNTAX__

typedef void *yyscan_t;

typedef struct syntax_init syntax_init, *p_syntax_init;

typedef struct syntax_funchead syntax_funchead, *p_syntax_funchead;

typedef struct syntax_decl syntax_decl, *p_syntax_decl;
typedef struct syntax_decl_list syntax_decl_list, *p_syntax_decl_list;

typedef struct syntax_param_decl syntax_param_decl, *p_syntax_param_decl;
typedef struct syntax_param_list syntax_param_list, *p_syntax_param_list;

#include <hir.h>

#endif