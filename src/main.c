#include <stdio.h>

#include <parser.h>
#include <lexer.h>

int main()
{
	yyscan_t scanner;
	int ret;
	yylex_init(&scanner);
	ret = yyparse(scanner);
	yylex_destroy(scanner);
	return ret;
}
