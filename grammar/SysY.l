/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <log.h>
#include "SysY.tab.h"
%}

%option yylineno batch noinput nounput noyywrap

LE   (<=)
GE   (>=)
EQ   (==)
NEQ  (!=)

AND  (&&)
OR   (\|\|)

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

%%

[+\-*/%&|~^,()\[\]{};<=>!] { yydebug("\"%s\" -> '%c'", yytext, yytext[0]); return yytext[0]; }

{LE}  { yydebug("\"%s\" -> LE", yytext);  return LE;  }
{GE}  { yydebug("\"%s\" -> GE", yytext);  return GE;  }
{EQ}  { yydebug("\"%s\" -> EQ", yytext);  return EQ;  }
{NEQ} { yydebug("\"%s\" -> NEQ", yytext); return NEQ; }

{AND} { yydebug("\"%s\" -> AND", yytext); return AND; }
{OR}  { yydebug("\"%s\" -> OR", yytext);  return OR;  }

{BLANK}+   { /* empty space */ }
{NEWLINE}+ { /* empty lines */ }

{INT}      { yydebug("\"%s\" -> INT", yytext);      return INT;      }
{FLOAT}    { yydebug("\"%s\" -> FLOAT", yytext);    return FLOAT;    }
{VOID}     { yydebug("\"%s\" -> VOID", yytext);     return VOID;     }
{CONST}    { yydebug("\"%s\" -> CONST", yytext);    return CONST;    }
{DO}       { yydebug("\"%s\" -> DO", yytext);       return DO;       }
{WHILE}    { yydebug("\"%s\" -> WHILE", yytext);    return WHILE;    }
{FOR}      { yydebug("\"%s\" -> FOR", yytext);      return FOR;      }
{BREAK}    { yydebug("\"%s\" -> BREAK", yytext);    return BREAK;    }
{CONTINUE} { yydebug("\"%s\" -> CONTINUE", yytext); return CONTINUE; }
{IF}       { yydebug("\"%s\" -> IF", yytext);       return IF;       }
{ELSE}     { yydebug("\"%s\" -> ELSE", yytext);     return ELSE;     }
{RETURN}   { yydebug("\"%s\" -> RETURN", yytext);   return RETURN;   }

{ID}       { return ID;       }

{HEXCONST} { yydebug("\"%s\" -> HEXCONST", yytext); return INTCONST; }
{DECCONST} { yydebug("\"%s\" -> DECCONST", yytext); return INTCONST; }
{OCTCONST} { yydebug("\"%s\" -> OCTCONST", yytext); return INTCONST; }

<<EOF>> { yydebug("<EOF>"); return 0; }

. { yyerror("%d no rules %s", yylineno, yytext); return 0; }
%%
