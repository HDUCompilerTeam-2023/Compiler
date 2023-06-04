#ifndef __MIR_VMEM__
#define __MIR_VMEM__

#include <mir.h>

/*
 * vmem type
 * pointer   ref_level > 0 && is_array = false && size = 1
 * variable  ref_level = 0 && is_array = false && size = 1
 * array     ref_level = 0 && is_array = true  && size = array_size
 *
 */
struct mir_vmem {
    p_symbol_var p_var;

    p_symbol_type p_type;

    size_t id;

    list_head node;
};

#endif
