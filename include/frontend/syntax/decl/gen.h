#ifndef __FRONTEND_SYNTAX_DECL_GEN__
#define __FRONTEND_SYNTAX_DECL_GEN__

#include <frontend/syntax/decl/use.h>

p_syntax_decl syntax_decl_gen(char *name);
void syntax_decl_drop(p_syntax_decl p_decl);

#endif
