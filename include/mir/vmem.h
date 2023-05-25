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
    basic_type b_type;

    size_t ref_level;
    bool is_array;
    size_t size;

    size_t id;

    list_head node;
};

#endif
