#include <stdio.h>

#include <parser.h>
#include <lexer.h>

int main(int argc, char *argv[])
{
	yyscan_t scanner;
	int ret = 0;

	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		yyopen_file(argv[i], &scanner);
		ret &= yyparse(scanner);
		yyclose_file(&scanner);
	}
	return ret;
}
