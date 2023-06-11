#ifndef __FRONTEND_SYNTAX_TYPE_USE__
#define __FRONTEND_SYNTAX_TYPE_USE__

#include <util.h>

#include <symbol/type.h>

typedef struct syntax_type_array syntax_type_array, *p_syntax_type_array;

p_symbol_type syntax_type_trans(p_syntax_type_array p_array, basic_type b_type);
p_syntax_type_array syntax_type_add_array(p_syntax_type_array p_array, p_syntax_type_array p_new_head);

#endif
