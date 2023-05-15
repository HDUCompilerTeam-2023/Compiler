#ifndef __IR_OPT_DEADCODE_ELIMATE__
#define __IR_OPT_DEADCODE_ELIMATE__

#include <ir.h>
#include <program/use.h>

void ir_deadcode_elimate_pass(p_program p_ir, bool if_aggressive);

#endif
