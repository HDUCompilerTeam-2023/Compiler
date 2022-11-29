#include <stdio.h>

#include <parser.h>
#include <lexer.h>
#include <frontend/syntaxtree_printer.h>

int main(int argc, char *argv[])
{
	yyscan_t scanner;
	int ret = 0;
	pCompUnitNode root;

	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		yyopen_file(argv[i], &scanner);
		ret &= yyparse(scanner, &root);
		frontend_print_syntaxtree(root);
		frontend_drop_syntaxtree(root);
		yyclose_file(&scanner);
	}
	return ret;
}
