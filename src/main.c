
#include <frontend.h>
#include <hir_print.h>
#include <hir_gen.h>
#include <hir2mir.h>
#include <mir_print.h>
int main(int argc, char *argv[])
{
	if (argc == 1)
		argv[argc++] = NULL;
	for (int i = 1; i < argc; ++i) {
		p_hir_program p_hir = frontend_trans(argv[i]);
		hir_program_print(p_hir);
		p_mir_program p_mir = hir2mir_program_gen(p_hir);
		mir_program_print(p_mir);
		hir_program_drop(p_hir);
		mir_program_drop(p_mir);
	}
	return 0;
}