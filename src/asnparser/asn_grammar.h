/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_ASN_GRAMMAR_H_INCLUDED
# define YY_YY_ASN_GRAMMAR_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     BIT_IDENTIFIER = 259,
     OID_IDENTIFIER = 260,
     IMPORT_IDENTIFIER = 261,
     MODULEREFERENCE = 262,
     TYPEREFERENCE = 263,
     OBJECTCLASSREFERENCE = 264,
     TYPEFIELDREFERENCE = 265,
     VALUEFIELDREFERENCE = 266,
     VALUESETFIELDREFERENCE = 267,
     OBJECTFIELDREFERENCE = 268,
     OBJECTSETFIELDREFERENCE = 269,
     OBJECTREFERENCE = 270,
     OBJECTSETREFERENCE = 271,
     INTEGER = 272,
     CSTRING = 273,
     OS_BSTRING = 274,
     BS_BSTRING = 275,
     OS_HSTRING = 276,
     BS_HSTRING = 277,
     STRING_BRACE = 278,
     BITSTRING_BRACE = 279,
     OID_BRACE = 280,
     ABSENT = 281,
     ABSTRACT_SYNTAX = 282,
     ALL = 283,
     ANY = 284,
     APPLICATION = 285,
     ASSIGNMENT = 286,
     AUTOMATIC = 287,
     BEGIN_t = 288,
     BIT = 289,
     BMPString = 290,
     BOOLEAN_t = 291,
     BY = 292,
     CHARACTER = 293,
     CHOICE = 294,
     CLASS = 295,
     COMPONENT = 296,
     COMPONENTS = 297,
     CONSTRAINED = 298,
     DEFAULT = 299,
     DEFINED = 300,
     DEFINITIONS = 301,
     EMBEDDED = 302,
     END = 303,
     ENUMERATED = 304,
     EXCEPT = 305,
     EXPLICIT = 306,
     EXPORTS = 307,
     EXTERNAL = 308,
     FALSE_t = 309,
     FROM = 310,
     GeneralString = 311,
     GraphicString = 312,
     IA5String = 313,
     TYPE_IDENTIFIER = 314,
     IDENTIFIER_t = 315,
     IMPLICIT = 316,
     IMPORTS = 317,
     INCLUDES = 318,
     INSTANCE = 319,
     INTEGER_t = 320,
     INTERSECTION = 321,
     ISO646String = 322,
     MACRO = 323,
     MAX_t = 324,
     MIN_t = 325,
     MINUS_INFINITY = 326,
     NOTATION = 327,
     NULL_VALUE = 328,
     NULL_TYPE = 329,
     NumericString = 330,
     OBJECT = 331,
     OCTET = 332,
     OF_t = 333,
     OPTIONAL_t = 334,
     PDV = 335,
     PLUS_INFINITY = 336,
     PRESENT = 337,
     PrintableString = 338,
     PRIVATE = 339,
     REAL = 340,
     SEQUENCE = 341,
     SET = 342,
     SIZE_t = 343,
     STRING = 344,
     SYNTAX = 345,
     T61String = 346,
     TAGS = 347,
     TeletexString = 348,
     TRUE_t = 349,
     TYPE_t = 350,
     UNION = 351,
     UNIQUE = 352,
     UNIVERSAL = 353,
     UniversalString = 354,
     VideotexString = 355,
     VisibleString = 356,
     GeneralizedTime = 357,
     UTCTime = 358,
     VALUE = 359,
     WITH = 360,
     string_t = 361,
     identifier_t = 362,
     number_t = 363,
     empty_t = 364,
     type_t = 365,
     value_t = 366,
     OBJECT_TYPE = 367,
     TRAP_TYPE = 368,
     ACCESS = 369,
     STATUS = 370,
     read_only_t = 371,
     read_write_t = 372,
     write_only_t = 373,
     not_accessible_t = 374,
     mandatory_t = 375,
     optional_t = 376,
     obsolete_t = 377,
     deprecated_t = 378,
     DESCRIPTION_t = 379,
     REFERENCE_t = 380,
     INDEX_t = 381,
     DEFVAL_t = 382,
     ENTERPRISE = 383,
     VARIABLES = 384,
     ObjectDescriptor_t = 385
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2053 of yacc.c  */
#line 403 "asn_grammar.y"

  PInt64	          ival;
  string	        * sval;
  list<string>	        * slst;
  TypeBase	        * tval;
  TypesList	        * tlst;
  ValueBase	        * vval;
  ValuesList            * vlst;
  NamedNumber	        * nval;
  NamedNumberList       * nlst;
  Constraint            * cons;
  ConstraintElementList * elst;
  ConstraintElementBase * elmt;
  struct {
    Tag::Type tagClass;
    unsigned tagNumber;
  } tagv;


/* Line 2053 of yacc.c  */
#line 207 "asn_grammar.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_ASN_GRAMMAR_H_INCLUDED  */
