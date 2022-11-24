/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <parser.h>
#include <frontend/log.h>
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

INT       (int)
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

HEXCONST  (0[xX][0-9a-fA-F]+)
OCTCONST  (0[0-7]*)
DECCONST  ([1-9][0-9]*)
LL_SUFFIX ("ll"|"LL")
INTSUFFIX ([lL][uU]?|[uU][lL]?|[uU]{LL_SUFFIX}|{LL_SUFFIX}[uU])
INTCONST  (({HEXCONST}|{OCTCONST}|{DECCONST}){INTSUFFIX}?)

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
    node->str = malloc(sizeof(yytext));
    strcpy(node->str, yytext);
    yylval->ID = node;
    return ID;
}

{INTCONST} {
    pINTCONSTNode node = malloc(sizeof(*node));
    node->type = type_int;
    char *endptr;
    node->val.val_int = strtol(yytext, &endptr, 0);
    if (*endptr != '\0') {
        yyerror(yylloc, yyscanner, "lexical error, wrong integer const, \"%s\"", yytext);
        exit(1);
    }
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
