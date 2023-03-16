/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <hir_gen/yyextra.h>
#include <hir_gen/parser.h>
#include <hir_gen/log.h>

#include <limits.h>

uint64_t str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner);
double str2float(const char *text, YYLTYPE *loc, yyscan_t scanner);
%}

%option reentrant bison-bridge bison-locations
%option yylineno batch noinput nounput noyywrap

%option extra-type="p_extra_info"

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
VOLATILE  (volatile)
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

HEXFLOATCONST  ((0[xX])(([0-9a-fA-F]*\.?[0-9a-fA-F]+)|([0-9a-fA-F]+\.))([pP][-+]?[0-9]+))
DECFLOATCONST  (([0-9]+[eE][-+]?[0-9]+)|((([0-9]+\.)|([0-9]*\.[0-9]+))([eE][-+]?[0-9]+)?))

%%

[-+*/%,()\[\]{};<=>!] { return yytext[0]; }

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
{VOLATILE} { return VOLATILE; }
{DO}       { return DO;       }
{WHILE}    { return WHILE;    }
{FOR}      { return FOR;      }
{BREAK}    { return BREAK;    }
{CONTINUE} { return CONTINUE; }
{IF}       { return IF;       }
{ELSE}     { return ELSE;     }
{RETURN}   { return RETURN;   }

{ID} {
    char *node = malloc(sizeof(*node) * (strlen(yytext) + 1));
    strcpy(node, yytext);
    yylval->ID = node;
    return ID;
}

{DECINTCONST} {
    uint64_t val = str2ull(yytext, 10, yylloc, yyscanner);

    yylval->INTCONST = val;
    return INTCONST;
}
{OCTINTCONST} {
    uint64_t val = str2ull(yytext, 8, yylloc, yyscanner);

    yylval->INTCONST = val;
    return INTCONST;
}
{HEXINTCONST} {
    uint64_t val = str2ull(yytext, 16, yylloc, yyscanner);

    yylval->INTCONST = val;
    return INTCONST;
}

{DECFLOATCONST} {
    double val = str2float(yytext, yylloc, yyscanner);
    yylval->FLOATCONST = val;
    return FLOATCONST;
}
{HEXFLOATCONST} {
    double val = str2float(yytext, yylloc, yyscanner);
    yylval->FLOATCONST = val;
    return FLOATCONST;
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
<header>[<][^\n>]+[>] { yytext[yyleng - 1] = '\0'; frontend_push_file(yytext + 1, yylloc, yyextra, yyscanner); BEGIN(INITIAL); }
<header>["][^\n"]+["] { yytext[yyleng - 1] = '\0'; frontend_push_file(yytext + 1, yylloc, yyextra, yyscanner); BEGIN(INITIAL); }

^[ \t]*#[ \t]*endfile { if (frontend_pop_file(yyextra, yyscanner)) yyterminate(); }
<<EOF>> { if (frontend_pop_file(yyextra, yyscanner)) yyterminate(); }

. { yyerror(yylloc, yyscanner, "lexical error, unexpected \"%s\"", yytext); }
%%
uint64_t str2ull(const char *text, int base, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    uint64_t tmp = strtoull(text, &endptr, base);
    if ((errno == ERANGE && tmp == ULLONG_MAX)
        || (errno == EINVAL && tmp == 0)) {
        yyerror(loc, scanner, "lexical error, wrong integer constant \"%s\": %m", text);
        tmp = 0;
        errno = 0;
    }
    if (*endptr != '\0') {
        yyerror(loc, scanner, "integer constant suffix \"%s\"", endptr);
    }
    return tmp;
}
double str2float(const char *text, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    double tmp = strtod(text, &endptr);
    if (errno == ERANGE) {
        yyerror(loc, scanner, "lexical error, wrong float constant \"%s\": %m", text);
        tmp = 0;
        errno = 0;
    }
    if (*endptr != '\0') {
        yyerror(loc, scanner, "float constant suffix \"%s\"", endptr);
    }
    return tmp;
}

