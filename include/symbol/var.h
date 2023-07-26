#ifndef __SYMBOL_VAR__
#define __SYMBOL_VAR__

#include <ir.h>
#include <symbol.h>
#include <program/use.h>

struct symbol_init_val {
    union {
        I32CONST_t i;
        F32CONST_t f;
    };
};

struct symbol_init {
    basic_type basic;
    size_t size;
    p_symbol_init_val memory;
};

struct symbol_var {
    // type info
    p_symbol_type p_type;

    char *name;
    uint64_t id;

    p_symbol_init p_init;
    bool is_global;
    bool is_const;

    union {
        p_program p_program;
        p_symbol_func p_func;
    };

    size_t stack_offset;
    list_head node;
};

#endif
