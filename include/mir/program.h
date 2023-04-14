
#ifndef __MIR_PROGRAM__
#define __MIR_PROGRAM__
#include <mir.h>
struct mir_program{
    p_list_head p_globalvar_head;
    list_head func;
};

#endif