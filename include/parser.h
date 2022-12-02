#ifndef __PARSER__
#define __PARSER__

#include <grammar/SysY.tab.h>

typedef struct file_stack *pfile_stack;
typedef struct ExtraInfo {
    pCompUnitNode root;
    pfile_stack fs;
} ExtraInfo, *pExtraInfo;

pExtraInfo frontend_init_extra();
void frontend_drop_extra(pExtraInfo ExtraInfo);

#ifndef __SYSY_LEX__
#define __SYSY_LEX__
#include <grammar/SysY.yy.h>
#endif

struct file_stack {
    pfile_stack prev;
    YY_BUFFER_STATE bs;
    YYLTYPE *loc;
    char *filename;
    FILE *f;
};

void yyopen_file(const char *file_name, YYLTYPE *loc, pExtraInfo extra, yyscan_t scanner);
bool yypop_file(pExtraInfo extra, yyscan_t scanner);

void yyinit(const char *file_name, yyscan_t *scanner);
void yyexit(yyscan_t *scanner);

#endif