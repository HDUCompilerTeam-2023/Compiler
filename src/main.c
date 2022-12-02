#include <stdio.h>

#include <parser.h>

int main(int argc, char *argv[])
{
	yyscan_t scanner;
	int ret = 0;

	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		yyinit(argv[i], &scanner);
		ret &= yyparse(scanner);
		yyexit(&scanner);
	}
	return ret;
}
