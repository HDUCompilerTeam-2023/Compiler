/*
 * SysY.l : Scanner for SysY language
 */

%{
#define __SYSY_LEX__
#include <limits.h>
#include <parser.h>
#include <frontend/log.h>

unsigned long long int str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner);
%}

%option reentrant bison-bridge bison-locations
%option yylineno batch noinput nounput noyywrap

%option extra-type="pExtraInfo"

%x MULTICOMMENT
%x LINECOMMENT

%x header

LE   (<=)
GE   (>=)
EQ   (==)
NEQ  (!=)

AND  (&&)
OR   (\|\|)

SELFADD ([+][+])
SELFSUB ([-][-])

UNSIGNED  (unsigned)
SIGNED    (signed)
SHORT     (short)
LONG      (long)
INT       (int)
CHAR      (char)
FLOAT     (float)
DOUBLE    (double)
VOID      (void)
CONST     (const)
DO        (do)
WHILE     (while)
FOR       (for)
BREAK     (break)
CONTINUE  (continue)
IF        (if)
ELSE      (else)
RETURN    (return)

BLANK     ([ \t\r\a])
NEWLINE   ([\n])

ID        ([_a-zA-Z][_a-zA-Z0-9]*)

HEXINTCONST  (0[xX][0-9a-fA-F]+)
OCTINTCONST  (0[0-7]*)
DECINTCONST  ([1-9][0-9]*)
ULL_SUFFIX ([uU]{LL_SUFFIX}|{LL_SUFFIX}[uU])
LL_SUFFIX ("ll"|"LL")
UL_SUFFIX ([uU][lL]|[lL][uU])
U_SUFFIX ([uU])
L_SUFFIX ([lL])

%%

[+\-*/%&|~^,()\[\]{};<=>!] { return yytext[0]; }

{SELFADD} { return SELFADD; }
{SELFSUB} { return SELFSUB; }

{LE}  { return LE;  }
{GE}  { return GE;  }
{EQ}  { return EQ;  }
{NEQ} { return NEQ; }

{AND} { return AND; }
{OR}  { return OR;  }

{BLANK}+   { /* empty space */ }
{NEWLINE}+ { /* empty lines */ }

{UNSIGNED} { return UNSIGNED; }
{SIGNED}   { return SIGNED;   }
{SHORT}    { return SHORT;    }
{LONG}     { return LONG;     }
{CHAR}     { return CHAR;     }
{INT}      { return INT;      }
{FLOAT}    { return FLOAT;    }
{DOUBLE}   { return DOUBLE;   }
{VOID}     { return VOID;     }
{CONST}    { return CONST;    }
{DO}       { return DO;       }
{WHILE}    { return WHILE;    }
{FOR}      { return FOR;      }
{BREAK}    { return BREAK;    }
{CONTINUE} { return CONTINUE; }
{IF}       { return IF;       }
{ELSE}     { return ELSE;     }
{RETURN}   { return RETURN;   }

{ID} {
    pIDNode node = malloc(strlen(yytext) + 1);
    strcpy(node, yytext);
    yylval->ID = node;
    return ID;
}

{DECINTCONST} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= INT_MAX) {
        node->valtype = type_int;
        node->val.val_int = val;
    }
    else if (val <= LONG_MAX) {
        node->valtype = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        errno = ERANGE;
        yyerror(yylloc, yyscanner, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
{DECINTCONST}{U_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= UINT_MAX) {
        node->valtype = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
{DECINTCONST}{L_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= LONG_MAX) {
        node->valtype = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= LLONG_MAX){
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else{
        errno = ERANGE;
        yyerror(yylloc, yyscanner, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
{DECINTCONST}{UL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
{DECINTCONST}{LL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);
    
    if (val <= LLONG_MAX) {
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        errno = ERANGE;
        yyerror(yylloc, yyscanner, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
{DECINTCONST}{ULL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    node->valtype = type_unsigned_long_long_int;
    node->val.val_unsigned_long_long_int = val;

    yylval->CONSTNUM = node;
    return CONSTNUM;
}

({OCTINTCONST}|{HEXINTCONST}) {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= INT_MAX) {
        node->valtype = type_int;
        node->val.val_int = val;
    }
    else if (val <= UINT_MAX) {
        node->valtype = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= LONG_MAX) {
        node->valtype = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
({OCTINTCONST}|{HEXINTCONST}){U_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= UINT_MAX) {
        node->valtype = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
({OCTINTCONST}|{HEXINTCONST}){L_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= LONG_MAX) {
        node->valtype = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
({OCTINTCONST}|{HEXINTCONST}){UL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= ULONG_MAX) {
        node->valtype = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
({OCTINTCONST}|{HEXINTCONST}){LL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= LLONG_MAX) {
        node->valtype = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->valtype = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->CONSTNUM = node;
    return CONSTNUM;
}
({OCTINTCONST}|{HEXINTCONST}){ULL_SUFFIX} {
    pCONSTNUMNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    node->valtype = type_unsigned_long_long_int;
    node->val.val_unsigned_long_long_int = val;

    yylval->CONSTNUM = node;
    return CONSTNUM;
}

[/][/] { BEGIN(LINECOMMENT); }
<LINECOMMENT>[^\\\n]+
<LINECOMMENT>[\\]+/[^\n]
<LINECOMMENT>([\\][\n])+
<LINECOMMENT>[\n] { BEGIN(INITIAL); }

[/][*] { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>[^*]+
<MULTICOMMENT>[*]+/[^/]
<MULTICOMMENT>[*][/] { BEGIN(INITIAL); }

^[ \t]*#[ \t]*include[ \t]* { BEGIN(header); }
<header>[<][^\n>]+[>] { yytext[yyleng - 1] = '\0'; yyopen_file(yytext + 1, yylloc, yyextra, yyscanner); BEGIN(INITIAL); }
<header>["][^\n"]+["] { yytext[yyleng - 1] = '\0'; yyopen_file(yytext + 1, yylloc, yyextra, yyscanner); BEGIN(INITIAL); }

^[ \t]*#[ \t]*endfile { if (yypop_file(yyextra, yyscanner)) yyterminate(); }
<<EOF>> { if (yypop_file(yyextra, yyscanner)) yyterminate(); }

. { yyerror(yylloc, yyscanner, "lexical error, unexpected \"%s\"", yytext); }
%%
unsigned long long int str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    unsigned long long int tmp = strtoull(text, &endptr, base);
    if ((errno == ERANGE && tmp == ULLONG_MAX)
        || (errno != 0 && tmp == 0)) {
        yyerror(loc, scanner, "lexical error, wrong integer constant \"%s\": %m", text);
        tmp = 0;
        errno = 0;
    }
    if (*endptr != '\0') {
        yydebug(loc, scanner, "integer constant suffix \"%s\"", endptr);
    }
    return tmp;
}

