#include <stdio.h>

#include <hir_gen.h>

int main(int argc, char *argv[])
{
	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		p_hir_program p_ast = hir_gen(argv[i]);
		hir_drop(p_ast);
	}
	return 0;
}