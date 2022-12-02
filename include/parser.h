#ifndef __PARSER__
#define __PARSER__

#include <grammar/SysY.tab.h>

typedef struct ExtraInfo {
    pCompUnitNode root;
} ExtraInfo, *pExtraInfo;

pExtraInfo frontend_init_extra();
void frontend_drop_extra(pExtraInfo ExtraInfo);

#ifndef __SYSY_LEX__
#define __SYSY_LEX__
#include <grammar/SysY.yy.h>
#endif


void yyopen_file(const char *file_name, yyscan_t *scanner);
void yyclose_file(yyscan_t *scanner);

#endif