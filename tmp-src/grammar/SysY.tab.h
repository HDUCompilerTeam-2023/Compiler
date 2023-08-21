/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_TMP_SRC_GRAMMAR_SYSY_TAB_H_INCLUDED
# define YY_YY_TMP_SRC_GRAMMAR_SYSY_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 24 "grammar/SysY.y"

#include <frontend/syntax/use.h>

#line 52 "tmp-src/grammar/SysY.tab.h"

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    UNSIGNED = 258,
    SIGNED = 259,
    LONG = 260,
    SHORT = 261,
    INT = 262,
    FLOAT = 263,
    DOUBLE = 264,
    CHAR = 265,
    VOID = 266,
    CONST = 267,
    VOLATILE = 268,
    DO = 269,
    WHILE = 270,
    FOR = 271,
    BREAK = 272,
    CONTINUE = 273,
    IF = 274,
    ELSE = 275,
    RETURN = 276,
    AND = 277,
    OR = 278,
    LE = 279,
    GE = 280,
    EQ = 281,
    NEQ = 282,
    SELFADD = 283,
    SELFSUB = 284,
    I32CONST = 285,
    F32CONST = 286,
    ID = 287,
    STRING = 288,
    NO_ELSE = 289
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 31 "grammar/SysY.y"

       p_ast_block p_block;
       p_ast_stmt p_stmt;
       p_ast_exp p_exp;

       p_ast_param_list p_param_list;

       p_syntax_decl p_decl;
       p_syntax_decl_head p_decl_head;

       p_syntax_init p_init;

       basic_type type;

       char *ID;
       char *STRING;
       I32CONST_t I32CONST;
       F32CONST_t F32CONST;

#line 118 "tmp-src/grammar/SysY.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (yyscan_t yyscanner);

#endif /* !YY_YY_TMP_SRC_GRAMMAR_SYSY_TAB_H_INCLUDED  */
