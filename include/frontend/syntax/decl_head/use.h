#ifndef __FRONTEND_SYNTAX_DECL_HEAD_USE__
#define __FRONTEND_SYNTAX_DECL_HEAD_USE__

#include <symbol/type.h>

typedef struct syntax_decl_head syntax_decl_head, *p_syntax_decl_head;

bool syntax_decl_head_get_is_const(p_syntax_decl_head p_list);
basic_type syntax_decl_head_get_basic(p_syntax_decl_head p_list);

#endif
