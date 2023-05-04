#include <frontend.h>
#include <hir2mir.h>

#include <hir_print.h>
#include <mir_print.h>

#include <mir_test.h>

int main(int argc, char *argv[])
{
	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		p_hir_program p_hir = frontend_trans(argv[i]);
		hir_program_print(p_hir);
		p_mir_program p_mir = hir2mir_program_gen(p_hir);
		mir_program_print(p_mir);
		mir_program_test(p_mir);
		mir_program_drop(p_mir);
	}
	return 0;
}