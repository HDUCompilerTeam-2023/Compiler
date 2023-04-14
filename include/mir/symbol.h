#ifndef __MIR_SYMBOL__
#define __MIR_SYMBOL__
#include <mir.h>
typedef enum{
    global_var, 
    local_var,
    temp_var,
}mir_sym_kind;
struct mir_symbol{
    union{
        size_t id; // 局部临时变量符号编号
        char *name; // 全局变量、函数符号, 是否需要 symbol 的全部信息？
    };
    mir_sym_kind kind;
    p_symbol_type p_type;
};

#endif