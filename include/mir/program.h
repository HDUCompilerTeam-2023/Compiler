
#ifndef __MIR_PROGRAM__
#define __MIR_PROGRAM__
#include <mir.h>
struct mir_program{
    list_head globalvar_init_list;// 全局变量初始化 ir
    list_head func;
};

#endif