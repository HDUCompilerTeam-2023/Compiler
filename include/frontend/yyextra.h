#ifndef __FRONTEND_YYEXTRA__
#define __FRONTEND_YYEXTRA__

#include <frontend/parser.h>
#include <frontend/symbol_table.h>

typedef struct extra_info extra_info, *p_extra_info;
typedef struct file_stack file_stack, *p_file_stack;

struct extra_info {
    p_file_stack fs;
    p_syntax_info p_info;
};

void frontend_push_file(const char *file_name, YYLTYPE *loc, p_extra_info extra, yyscan_t scanner);
bool frontend_pop_file(p_extra_info extra, yyscan_t scanner);

#endif
