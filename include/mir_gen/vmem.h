#ifndef __MIR_GEN_VMEM__
#define __MIR_GEN_VMEM__

#include <mir/vmem.h>

p_mir_vmem mir_vmem_temp_gen(basic_type b_type, size_t ref_level);
p_mir_vmem mir_vmem_sym_gen(p_symbol_sym p_sym);
void mir_vmem_drop(p_mir_vmem p_vmem);

#endif
