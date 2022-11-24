#include <stdio.h>

#include <parser.h>
#include <lexer.h>

int main()
{
	yyscan_t scanner;
	int ret;
	yylex_init(&scanner);
	pCompUnitNode root;
	ret = yyparse(scanner, &root);
	yylex_destroy(scanner);
	return ret;
}
