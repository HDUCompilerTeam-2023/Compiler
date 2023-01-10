#ifndef __FRONTEND_TYPE__
#define __FRONTEND_TYPE__

#include <util.h>

typedef enum basictype {
    type_void, // void
    type_char, // char  signed char
    type_unsigned_char, // unsigned char
    type_short_int, // short  signed short  short int  signed short int
    type_unsigned_short_int, // unsigned short  unsigned short int
    type_int, // int  signed  signed int
    type_unsigned_int, // unsigned  unsigned int
    type_long_int, // long  signed long  long int  signed long int
    type_unsigned_long_int, // unsigned long  unsigned long int
    type_long_long_int, // long long  signed long long  long long int  signed long long int
    type_unsigned_long_long_int, // unsigned long long  unsigned long long int
    type_float, // float
    type_double, // double
    type_long_double, // long double
} basictype;

typedef struct type type, *ptype;
struct type {
    union {
        struct {
            basictype basic: 4;
        } basic;
        struct {
            ptype base;
        } arrary;
        struct {
            ptype base;
        } point;
        struct {
            ptype *types; // length - size + 1, types[0] - ret type, types[1] ~ types[size] - arg type
        } func;
    } select;
    uint32_t size;
    enum {
        basic,
        arrary,
        point,
        func,
    } type: 2;
    bool const_flag: 1;
    bool volatile_flag: 1;
};

#endif
