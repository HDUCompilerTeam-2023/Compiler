#include <ir_opt/lir_gen/arm_standard.h>
#include <symbol/type.h>

size_t caller_save_reg_r[caller_save_reg_num_r] = { 0, 1, 2, 3, 12 };
size_t callee_save_reg_r[callee_save_reg_num_r] = { 4, 5, 6, 7, 8, 9, 10, 11 };
size_t caller_save_reg_s[caller_save_reg_num_s] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
size_t callee_save_reg_s[callee_save_reg_num_s] = { 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 };

size_t temp_reg_r[temp_reg_num_r] = { 0, 1, 2, 3 };
size_t temp_reg_s[temp_reg_num_s] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

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

bool if_caller_save_reg(size_t id) {
    for (size_t i = 0; i < caller_save_reg_num_r; i++) {
        if (caller_save_reg_r[i] == id)
            return true;
    }
    for (size_t i = 0; i < caller_save_reg_num_s; i++) {
        if (caller_save_reg_s[i] == id)
            return true;
    }
    return false;
}