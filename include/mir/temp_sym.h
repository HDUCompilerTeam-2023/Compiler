#ifndef __MIR_TEMP_SYM__
#define __MIR_TEMP_SYM__

#include <mir.h>
struct mir_temp_sym{
    bool is_pointer;
    basic_type b_type;
    size_t id;

    list_head node;
};

#endif