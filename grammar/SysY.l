/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <log.h>
%}

%option yylineno batch noinput nounput noyywrap

INT (int)
FLOAT (float)
VOID (void)
CONST (const)
DO (do)
WHILE (while)
FOR (for)
BREAK (break)
CONTINUE (continue)
IF (if)
ELSE (else)
RETURN (return)

LE (<=)
GE (>=)
EQ (==)
NEQ (!=)

AND (&&)
OR (\|\|)

BLANK ([ \t\r\a])
NEWLINE ([\n])

ID ([_a-zA-Z][_a-zA-Z0-9]*)

HEXCONST (0[xX][0-9a-fA-F]+)
OCTCONST (0[0-7]*)
DECCONST ([1-9][0-9]*)

%%

[+\-*/%&|~^,()\[\]{};<=>!] { printf("%d '%s'\n", yylineno, yytext); }

{LE}  { /* return LE;  */ printf("%d LE\n", yylineno); }
{GE}  { /* return GE;  */ printf("%d GE\n", yylineno); }
{EQ}  { /* return EQ;  */ printf("%d EQ\n", yylineno); }
{NEQ} { /* return NEQ; */ printf("%d NEQ\n", yylineno); }

{AND} { /* return AND */ printf("%d AND\n", yylineno); }
{OR}  { /* return OR  */ printf("%d OR\n", yylineno); }

{BLANK}+   { /* empty space */ }
{NEWLINE}+ { /* empty lines */ }

{INT}      { printf("%d INT\n", yylineno);      }
{FLOAT}    { printf("%d FLOAT\n", yylineno);    }
{VOID}     { printf("%d VOID\n", yylineno);     }
{CONST}    { printf("%d CONST\n", yylineno);    }
{DO}       { printf("%d DO\n", yylineno);       }
{WHILE}    { printf("%d WHILE\n", yylineno);    }
{FOR}      { printf("%d FOR\n", yylineno);      }
{BREAK}    { printf("%d BREAK\n", yylineno);    }
{CONTINUE} { printf("%d CONTINUE\n", yylineno); }
{IF}       { printf("%d IF\n", yylineno);       }
{ELSE}     { printf("%d ELSE\n", yylineno);     }
{RETURN}   { printf("%d RETURN\n", yylineno);   }

{ID} { printf("%d ID %s\n", yylineno, yytext); }

{HEXCONST} { printf("%d HEXCONST %s\n", yylineno, yytext); }
{DECCONST} { printf("%d DECCONST %s\n", yylineno, yytext); }
{OCTCONST} { printf("%d OCTCONST %s\n", yylineno, yytext); }

<<EOF>> { return 0; }

. { yyerror("%d no rules %s", yylineno, yytext); return 0; }
%%
