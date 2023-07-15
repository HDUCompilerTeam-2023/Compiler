#ifndef __FRONTEND_SYNTAX_INFO_GEN__
#define __FRONTEND_SYNTAX_INFO_GEN__

#include <frontend/syntax/info/use.h>

p_syntax_info syntax_info_gen(const char *input, const char *output);
void syntax_info_drop(p_syntax_info p_info);

#endif
