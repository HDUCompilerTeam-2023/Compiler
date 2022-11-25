/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <limits.h>
#include <parser.h>
#include <frontend/log.h>

unsigned long long int str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner);
%}

%option reentrant bison-bridge bison-locations
%option yylineno batch noinput nounput noyywrap

%x MULTICOMMENT
%x LINECOMMENT

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
    pIDNode node = malloc(sizeof(*node));
    node->str = malloc(strlen(yytext) + 1);
    strcpy(node->str, yytext);
    yylval->ID = node;
    return ID;
}

{DECINTCONST} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= INT_MAX) {
        node->type = type_int;
        node->val.val_int = val;
    }
    else if (val <= LONG_MAX) {
        node->type = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        errno = ERANGE;
        yyerror(yylloc, yyscanner, NULL, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
{DECINTCONST}{U_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= UINT_MAX) {
        node->type = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
{DECINTCONST}{L_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= LONG_MAX) {
        node->type = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= LLONG_MAX){
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else{
        errno = ERANGE;
        yyerror(yylloc, yyscanner, NULL, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
{DECINTCONST}{UL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
{DECINTCONST}{LL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);
    
    if (val <= LLONG_MAX) {
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        errno = ERANGE;
        yyerror(yylloc, yyscanner, NULL, "lexical error, wrong integer constant \"%s\": %m", yytext);
        errno = 0;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
{DECINTCONST}{ULL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 10, yylloc, yyscanner);

    node->type = type_unsigned_long_long_int;
    node->val.val_unsigned_long_long_int = val;

    yylval->INTCONST = node;
    return INTCONST;
}

({OCTINTCONST}|{HEXINTCONST}) {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= INT_MAX) {
        node->type = type_int;
        node->val.val_int = val;
    }
    else if (val <= UINT_MAX) {
        node->type = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= LONG_MAX) {
        node->type = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
({OCTINTCONST}|{HEXINTCONST}){U_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= UINT_MAX) {
        node->type = type_unsigned_int;
        node->val.val_unsigned_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
({OCTINTCONST}|{HEXINTCONST}){L_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= LONG_MAX) {
        node->type = type_long_int;
        node->val.val_long_int = val;
    }
    else if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else if (val <= LLONG_MAX) {
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
({OCTINTCONST}|{HEXINTCONST}){UL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= ULONG_MAX) {
        node->type = type_unsigned_long_int;
        node->val.val_unsigned_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
({OCTINTCONST}|{HEXINTCONST}){LL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    if (val <= LLONG_MAX) {
        node->type = type_long_long_int;
        node->val.val_long_long_int = val;
    }
    else {
        node->type = type_unsigned_long_long_int;
        node->val.val_unsigned_long_long_int = val;
    }

    yylval->INTCONST = node;
    return INTCONST;
}
({OCTINTCONST}|{HEXINTCONST}){ULL_SUFFIX} {
    pINTCONSTNode node = malloc(sizeof(*node));

    unsigned long long int val = str2ull(yytext, 0, yylloc, yyscanner);

    node->type = type_unsigned_long_long_int;
    node->val.val_unsigned_long_long_int = val;

    yylval->INTCONST = node;
    return INTCONST;
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

<<EOF>> { yyterminate(); }

. { yyerror(yylloc, yyscanner, NULL, "lexical error, unexpected \"%s\"", yytext); }
%%
unsigned long long int str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    unsigned long long int tmp = strtoull(text, &endptr, base);
    if ((errno == ERANGE && tmp == ULLONG_MAX)
        || (errno != 0 && tmp == 0)) {
        yyerror(loc, scanner, NULL, "lexical error, wrong integer constant \"%s\": %m", text);
        tmp = 0;
        errno = 0;
    }
    if (*endptr != '\0') {
        yydebug(loc, scanner, "integer constant suffix \"%s\"", endptr);
    }
    return tmp;
}

