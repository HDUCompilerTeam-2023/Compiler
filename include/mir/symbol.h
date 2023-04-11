#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>
struct mir_symbol{
    union{
        size_t id; // 局部、临时变量符号编号
        char *global_name; // 全局变量符号
    };
    enum{
        global_var, 
        local_var,
        temp_var,
    }irsym_kind;
    p_symbol_type p_type;
};

struct mir_operand{
    union{
        p_mir_symbol p_sym;
        BasicVal immediate_val; // 是否进行封装 ？
    };
    bool is_immediate;
};

// 把两个进行合并的实现：

// struct ir_operand{
//     union{
//         size_t id; // 局部、临时变量符号编号
//         char *global_name; // 全局变量符号
//         int intconst;
//         float floatconst;
//     };
//     enum{
//         global_var, 
//         local_var,
//         temp_var,
//         immediate_val,
//     }irsym_kind;
//     union{
//         p_symbol_type p_type;
//         basic_type basictype;
//     }
// };



#endif