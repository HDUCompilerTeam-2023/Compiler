#include <ir_opt/lir_gen/arm_standard.h>
#include <symbol/type.h>
// value循环右移bits位
#define ror(value, bits) ((value >> bits) | (value << (sizeof(value) * 8 - bits)))

// 是否由八位循环右移偶数位得到
bool if_legal_rotate_imme12(I32CONST_t i32const) {
    if (i32const == 0) return true;
    if (i32const < 0) // 负数
        i32const = -i32const;
    uint32_t trans = i32const;
    uint32_t window = ~imm_8_max;
    for (size_t i = 0; i < 16; i++) {
        if (!(window & trans))
            return true;
        window = ror(window, 2);
    }
    return false;
}

bool if_legal_direct_imme12(I32CONST_t i32const) {
    return !(i32const > imm_12_max || i32const < -imm_12_max);
}

bool if_legal_direct_imme8(I32CONST_t i32const) {
    return !(i32const > imm_8_max || i32const < -imm_8_max);
}

bool if_in_r(p_symbol_type p_type) {
    if (p_type->ref_level > 0)
        return true;
    switch (p_type->basic) {
    case type_i32:
    case type_str:
        return true;
    case type_f32:
        return false;
    case type_void:
        assert(0);
    }
    return false;
}