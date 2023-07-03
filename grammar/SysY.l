/*
 * SysY.l : Scanner for SysY language
 */

%{
#include <frontend/parser.h>
#include <frontend/log.h>

#include <limits.h>

I32CONST_t str2int(const char *text, int base, YYLTYPE *loc, yyscan_t scanner);
F32CONST_t str2float(const char *text, YYLTYPE *loc, yyscan_t scanner);
YY_BUFFER_STATE yy_create_buffer_string (const char * yystr , yyscan_t yyscanner);

#define YY_USER_ACTION \
do { \
    yylloc->first_column = yylloc->last_column; \
    yylloc->last_column = yylloc->last_column + yyleng; \
    yylloc->first_line = yylloc->last_line; \
    for (size_t i = 0; i < yyleng; ++i) { \
        if (yytext[i] == '\n') { \
            ++yylloc->last_line; \
            yylloc->last_column = 1; \
        } \
    } \
} while (0);
%}

%option reentrant bison-bridge bison-locations
%option yylineno batch noinput nounput noyywrap

%option extra-type="p_syntax_info"

%x MULTICOMMENT
%x LINECOMMENT

%x header

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
WHILE     (while)
BREAK     (break)
CONTINUE  (continue)
IF        (if)
ELSE      (else)
RETURN    (return)

BLANK     ([ \t\r\a])
NEWLINE   ([\n])

ID        ([_a-zA-Z][_a-zA-Z0-9]*)

HEXI32CONST  (0[xX][0-9a-fA-F]+)
OCTI32CONST  (0[0-7]*)
DECI32CONST  ([1-9][0-9]*)

HEXFLOATCONST  ((0[xX])(([0-9a-fA-F]*\.?[0-9a-fA-F]+)|([0-9a-fA-F]+\.))([pP][-+]?[0-9]+))
DECFLOATCONST  (([0-9]+[eE][-+]?[0-9]+)|((([0-9]+\.)|([0-9]*\.[0-9]+))([eE][-+]?[0-9]+)?))

STRING (["][^"\n]*["])

MACRO_STARTTIME "starttime"({BLANK}|{NEWLINE})*"()"
MACRO_STOPTIME  "stoptime"({BLANK}|{NEWLINE})*"()"
MACRO_LINE      "__LINE__"

%%

[-+*/%,()\[\]{};<=>!] { return yytext[0]; }

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
{WHILE}    { return WHILE;    }
{BREAK}    { return BREAK;    }
{CONTINUE} { return CONTINUE; }
{IF}       { return IF;       }
{ELSE}     { return ELSE;     }
{RETURN}   { return RETURN;   }

{MACRO_STARTTIME} {
    yypush_buffer_state(yy_create_buffer_string("_sysy_starttime(__LINE__)", yyscanner), yyscanner);
}
{MACRO_STOPTIME} {
    yypush_buffer_state(yy_create_buffer_string("_sysy_stoptime(__LINE__)", yyscanner), yyscanner);
}
{MACRO_LINE} {
    I32CONST_t val = yylloc->first_line;

    yylval->I32CONST = val;
    return I32CONST;
}

{ID} {
    char *node = malloc(sizeof(*node) * (strlen(yytext) + 1));
    strcpy(node, yytext);
    yylval->ID = node;
    return ID;
}

{DECI32CONST} {
    I32CONST_t val = str2int(yytext, 10, yylloc, yyscanner);

    yylval->I32CONST = val;
    return I32CONST;
}
{OCTI32CONST} {
    I32CONST_t val = str2int(yytext, 8, yylloc, yyscanner);

    yylval->I32CONST = val;
    return I32CONST;
}
{HEXI32CONST} {
    I32CONST_t val = str2int(yytext, 16, yylloc, yyscanner);

    yylval->I32CONST = val;
    return I32CONST;
}

{DECFLOATCONST} {
    F32CONST_t val = str2float(yytext, yylloc, yyscanner);
    yylval->F32CONST = val;
    return F32CONST;
}
{HEXFLOATCONST} {
    F32CONST_t val = str2float(yytext, yylloc, yyscanner);
    yylval->F32CONST = val;
    return F32CONST;
}

{STRING} {
    size_t length = strlen(yytext) - 2;
    char *str = malloc(sizeof(*str) * length + 1);
    strncpy(str, yytext + 1, length);
    str[length] = '\0';
    yylval->STRING = str;
    return STRING;
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

^[ \t]*#[ \t]*endfile { yyterminate(); }
<<EOF>> {
    yypop_buffer_state(yyscanner);
    if (!YY_CURRENT_BUFFER) yyterminate();
}

. { yyerror(yylloc, yyscanner, "lexical error, unexpected \"%s\"", yytext); }
%%
I32CONST_t str2int(const char *text, int base, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    long long tmp = strtoll(text, &endptr, base);
    if ((errno == ERANGE && tmp == LLONG_MAX)) {
        yyerror(loc, scanner, "lexical error, integer constant too big\"%s\": %m", text);
        tmp = 0;
        errno = 0;
    }
    I32CONST_t ret = tmp;
    if (*endptr != '\0') {
        yyerror(loc, scanner, "integer constant suffix \"%s\"", endptr);
    }
    return ret;
}
F32CONST_t str2float(const char *text, YYLTYPE *loc, yyscan_t scanner) {
    char *endptr;
    errno = 0;
    F32CONST_t tmp = strtod(text, &endptr);
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

YY_BUFFER_STATE yy_create_buffer_buffer  (char * str, yy_size_t size, yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;

	if ( size < 2 ||
	     str[size-2] != YY_END_OF_BUFFER_CHAR ||
	     str[size-1] != YY_END_OF_BUFFER_CHAR )
		/* They forgot to leave room for the EOB's. */
		return NULL;

	b = (YY_BUFFER_STATE) yyalloc( sizeof( struct yy_buffer_state ) , yyscanner );
	if ( ! b )
		YY_FATAL_ERROR( "out of dynamic memory in yy_scan_buffer()" );

	b->yy_buf_size = (int) (size - 2);	/* "- 2" to take care of EOB's */
	b->yy_buf_pos = b->yy_ch_buf = str;
	b->yy_is_our_buffer = 0;
	b->yy_input_file = NULL;
	b->yy_n_chars = b->yy_buf_size;
	b->yy_is_interactive = 0;
	b->yy_at_bol = 1;
	b->yy_fill_buffer = 0;
	b->yy_buffer_status = YY_BUFFER_NEW;

	return b;
}
YY_BUFFER_STATE yy_create_buffer_bytes  (const char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	int i;

	/* Get memory for full buffer, including space for trailing EOB's. */
	n = (yy_size_t) (_yybytes_len + 2);
	buf = (char *) yyalloc( n , yyscanner );
	if ( ! buf )
		YY_FATAL_ERROR( "out of dynamic memory in yy_scan_bytes()" );

	for ( i = 0; i < _yybytes_len; ++i )
		buf[i] = yybytes[i];

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;

	b = yy_create_buffer_buffer( buf, n , yyscanner);
	if ( ! b )
		YY_FATAL_ERROR( "bad buffer in yy_scan_bytes()" );

	/* It's okay to grow etc. this buffer, and we should throw it
	 * away when we're done.
	 */
	b->yy_is_our_buffer = 1;

	return b;
}
YY_BUFFER_STATE yy_create_buffer_string (const char * yystr , yyscan_t yyscanner)
{
	return yy_create_buffer_bytes( yystr, (int) strlen(yystr) , yyscanner);
}

