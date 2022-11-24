#include <stdio.h>

#include <parser.h>
#include <lexer.h>
#include <frontend/syntaxtree_printer.h>

int main()
{
	yyscan_t scanner;
	int ret;
	yylex_init(&scanner);
	pCompUnitNode root;
	ret = yyparse(scanner, &root);
	frontend_print_syntaxtree(root);
	yylex_destroy(scanner);
	return ret;
}
