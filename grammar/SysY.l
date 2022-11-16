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

[+\-*/%&|~^,()\[\]{};<=>!] { return yytext[0]; }

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

{ID}       { return ID;       }

{HEXCONST} { return INTCONST; }
{DECCONST} { return INTCONST; }
{OCTCONST} { return INTCONST; }

<<EOF>> { return 0; }

. { yyerror("%d no rules %s", yylineno, yytext); return 0; }
%%
