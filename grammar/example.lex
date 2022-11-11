/*
 * labLexer-2.lex : Scanner for a simple
 *									relop parser.
 */

%{
#include <stdio.h>
%}

%%

>=|<=|<> {
  printf("(relop,%s)", yytext + yyleng - 2);
}
[<=>] {
  printf("(relop,%s)", yytext + yyleng - 1);
}
\n|\r\n {
	return 0;
}
[^<=>\n]+ {
	char c = input();
	unput(c);
	if ( c == '\n' && yytext[yyleng - 1] == '\r' ) REJECT;
  printf("(other,%d)", (int)yyleng);
}
. {
  printf("(err,%s)", yytext);
	return 1;
}

%%
