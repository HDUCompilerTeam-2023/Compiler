#ifndef __PARSER__
#define __PARSER__

#include <grammar/SysY.tab.h>

typedef struct ExtraInfo {
    pCompUnitNode root;
} ExtraInfo, *pExtraInfo;

pExtraInfo frontend_init_extra();
void frontend_drop_extra(pExtraInfo ExtraInfo);

#endif