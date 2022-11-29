#ifndef __LEXER__
#define __LEXER__

#include <parser.h>
#include <grammar/SysY.yy.h>

void yyopen_file(const char *file_name, yyscan_t *scanner);
void yyclose_file(yyscan_t *scanner);

#endif