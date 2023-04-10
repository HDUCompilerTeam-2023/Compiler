#include <stdio.h>

#include <frontend.h>
#include <hir_print.h>
#include <hir_gen.h>

int main(int argc, char *argv[])
{
	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		p_hir_program p_hir = frontend_trans(argv[i]);
		hir_program_print(p_hir);
		hir_program_drop(p_hir);
	}
	return 0;
}