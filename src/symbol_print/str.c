#include <symbol_print.h>
#include <stdio.h>

#include <symbol/str.h>

void symbol_str_print(p_symbol_str p_str) {
    printf("\"%s\"", p_str->string);
}