#ifndef __HIR_TYPE__
#define __HIR_TYPE__

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

#endif
