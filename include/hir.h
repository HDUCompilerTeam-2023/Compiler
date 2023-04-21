#ifndef __HIR__
#define __HIR__

#include <util.h>
#include <symbol.h>

typedef int64_t INTCONST_t;
typedef double FLOATCONST_t;
typedef char *STRING_t;
typedef char *ID_t;

typedef struct hir_program hir_program, *p_hir_program;
typedef struct hir_param hir_param, *p_hir_param;
typedef struct hir_param_list hir_param_list, *p_hir_param_list;
typedef struct hir_func hir_func, *p_hir_func;
typedef struct hir_block hir_block, *p_hir_block;
typedef struct hir_stmt hir_stmt, *p_hir_stmt;
typedef struct hir_exp hir_exp, *p_hir_exp;
typedef enum hir_exp_op hir_exp_op;

#endif