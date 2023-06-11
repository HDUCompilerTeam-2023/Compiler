#ifndef __FRONTEND_SYNTAX_DECL_LIST_USE__
#define __FRONTEND_SYNTAX_DECL_LIST_USE__

#include <util.h>
#include <symbol/type.h>
#include <frontend/syntax/decl_list/node/use.h>

typedef struct syntax_decl_list syntax_decl_list, *p_syntax_decl_list;

bool syntax_decl_list_get_is_const(p_syntax_decl_list p_list);
p_list_head syntax_decl_list_get_list(p_syntax_decl_list p_list);
basic_type syntax_decl_list_get_basic(p_syntax_decl_list p_list);

p_syntax_decl_list syntax_decl_list_add(p_syntax_decl_list p_decl_list, p_syntax_decl p_decl);
p_syntax_decl_list syntax_decl_list_set(p_syntax_decl_list p_decl_list, bool is_const, basic_type type);

#endif
