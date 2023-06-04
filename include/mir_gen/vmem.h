#ifndef __MIR_GEN_VMEM__
#define __MIR_GEN_VMEM__

#include <mir/vmem.h>

p_mir_vmem mir_vmem_temp_gen(p_symbol_type p_type);
p_mir_vmem mir_vmem_sym_gen(p_symbol_var p_var);
void mir_vmem_drop(p_mir_vmem p_vmem);

#endif
