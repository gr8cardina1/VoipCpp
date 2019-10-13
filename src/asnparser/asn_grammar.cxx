/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7.12-4996"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 3 "asn_grammar.y"


#include <ptlib.h>

#undef malloc
#undef calloc
#undef realloc
#undef free

#include "main.h"

extern int yylex();

extern int ReferenceTokenContext;
extern int IdentifierTokenContext;
extern int BraceTokenContext;
extern int NullTokenContext;
extern int InMacroContext;
extern int HasObjectTypeMacro;
extern int InMIBContext;
extern TypesList * CurrentImportList;

static int UnnamedFieldCount = 1;
static list<string> * DummyParameters;

static string * ConcatNames(string * s1, char c, string * s2)
{
  *s1 += c;
  *s1 += *s2;
  delete s2;
  return s1;
}

#ifdef _MSC_VER
#pragma warning(disable:4701)
#endif


/* Line 371 of yacc.c  */
#line 182 "asn_grammar"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "asn_grammar.h".  */
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
/* Line 387 of yacc.c  */
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


/* Line 387 of yacc.c  */
#line 375 "asn_grammar"
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

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 403 "asn_grammar"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif


/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3768

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  149
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  242
/* YYNRULES -- Number of rules.  */
#define YYNRULES  535
/* YYNRULES -- Number of states.  */
#define YYNSTATES  852

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   385

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   141,     2,     2,     2,     2,     2,     2,
     133,   134,     2,     2,   136,   148,   137,     2,   146,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   142,   135,
     140,     2,   147,     2,   145,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   138,     2,   139,   144,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   131,   143,   132,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,    14,    18,    19,    21,    24,    26,
      28,    30,    35,    38,    41,    44,    45,    49,    50,    54,
      55,    57,    58,    62,    63,    65,    66,    68,    71,    72,
      77,    78,    82,    84,    86,    87,    89,    91,    93,    97,
      99,   101,   103,   106,   108,   110,   112,   114,   116,   118,
     120,   122,   124,   126,   127,   133,   137,   139,   141,   143,
     145,   147,   149,   151,   153,   155,   157,   159,   161,   163,
     165,   167,   169,   171,   173,   175,   177,   179,   181,   183,
     185,   187,   189,   191,   193,   195,   197,   201,   204,   210,
     212,   216,   221,   226,   228,   230,   232,   234,   236,   238,
     240,   242,   244,   246,   248,   250,   252,   254,   256,   259,
     264,   266,   270,   276,   278,   282,   287,   290,   292,   295,
     300,   302,   308,   316,   318,   322,   324,   326,   328,   330,
     335,   339,   341,   346,   348,   352,   355,   358,   360,   365,
     369,   374,   376,   380,   386,   390,   392,   396,   398,   401,
     402,   407,   411,   415,   420,   424,   428,   431,   435,   439,
     444,   446,   448,   450,   452,   454,   455,   459,   461,   463,
     465,   469,   471,   473,   475,   477,   480,   483,   485,   490,
     495,   500,   505,   510,   512,   514,   517,   518,   520,   522,
     526,   528,   534,   540,   547,   549,   552,   554,   558,   560,
     564,   566,   569,   572,   574,   576,   578,   580,   582,   584,
     588,   590,   592,   594,   596,   598,   600,   605,   607,   610,
     612,   615,   617,   619,   621,   623,   626,   629,   631,   633,
     635,   637,   640,   644,   648,   652,   660,   662,   666,   669,
     673,   675,   677,   679,   680,   682,   684,   690,   691,   693,
     697,   699,   703,   705,   707,   709,   716,   720,   722,   725,
     729,   733,   735,   739,   744,   749,   751,   753,   755,   757,
     759,   761,   765,   767,   769,   775,   779,   781,   783,   785,
     787,   789,   791,   793,   795,   798,   800,   803,   804,   809,
     811,   812,   814,   817,   818,   822,   826,   828,   831,   832,
     836,   840,   842,   845,   846,   850,   852,   855,   856,   860,
     861,   865,   868,   871,   873,   875,   877,   881,   883,   885,
     887,   889,   891,   893,   895,   897,   899,   901,   903,   905,
     907,   909,   911,   913,   915,   917,   919,   921,   923,   925,
     927,   929,   931,   933,   935,   939,   942,   944,   946,   950,
     953,   957,   959,   961,   963,   965,   967,   969,   971,   973,
     975,   979,   983,   985,   987,   989,   991,   993,   995,   997,
     999,  1001,  1002,  1008,  1014,  1020,  1025,  1031,  1037,  1041,
    1045,  1047,  1051,  1053,  1055,  1057,  1060,  1062,  1064,  1068,
    1072,  1074,  1076,  1078,  1080,  1082,  1083,  1089,  1091,  1093,
    1095,  1097,  1099,  1101,  1103,  1105,  1107,  1109,  1111,  1113,
    1115,  1117,  1119,  1123,  1124,  1129,  1131,  1134,  1136,  1138,
    1143,  1145,  1147,  1149,  1151,  1153,  1155,  1157,  1161,  1164,
    1166,  1170,  1172,  1174,  1176,  1178,  1180,  1182,  1184,  1188,
    1190,  1194,  1196,  1198,  1208,  1214,  1218,  1220,  1222,  1224,
    1226,  1228,  1230,  1234,  1237,  1239,  1243,  1246,  1248,  1251,
    1253,  1255,  1259,  1264,  1265,  1270,  1272,  1276,  1279,  1284,
    1289,  1291,  1295,  1297,  1300,  1302,  1304,  1306,  1308,  1311,
    1313,  1315,  1317,  1319,  1321,  1327,  1332,  1338,  1344,  1348,
    1350,  1353,  1355,  1357,  1361,  1366,  1367,  1368,  1385,  1387,
    1389,  1391,  1393,  1395,  1397,  1399,  1401,  1404,  1405,  1408,
    1409,  1414,  1415,  1417,  1421,  1423,  1425,  1430,  1431,  1432,
    1443,  1448,  1449,  1451,  1455,  1457,  1459,  1461,  1463,  1465,
    1469,  1471,  1475,  1480,  1485,  1487
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     150,     0,    -1,    -1,     7,   152,    46,   156,    31,    33,
     151,   157,    48,    -1,   131,   153,   132,    -1,    -1,   154,
      -1,   154,   153,    -1,     3,    -1,    17,    -1,   155,    -1,
       3,   133,    17,   134,    -1,    51,    92,    -1,    61,    92,
      -1,    32,    92,    -1,    -1,   158,   160,   171,    -1,    -1,
      52,   159,   135,    -1,    -1,   169,    -1,    -1,    62,   161,
     135,    -1,    -1,   162,    -1,    -1,   163,    -1,   162,   163,
      -1,    -1,   169,    55,   164,   165,    -1,    -1,     7,   166,
     167,    -1,   168,    -1,   328,    -1,    -1,   327,    -1,     6,
      -1,   170,    -1,   170,   136,   169,    -1,   386,    -1,   387,
      -1,   172,    -1,   171,   172,    -1,   175,    -1,   322,    -1,
     173,    -1,   263,    -1,   264,    -1,   265,    -1,   305,    -1,
     356,    -1,   371,    -1,   382,    -1,    -1,     8,   176,   174,
      31,   355,    -1,     8,    31,   176,    -1,   224,    -1,   178,
      -1,   177,    -1,   181,    -1,   184,    -1,   185,    -1,   188,
      -1,   193,    -1,   194,    -1,   198,    -1,   199,    -1,   200,
      -1,   201,    -1,   202,    -1,   203,    -1,   204,    -1,   205,
      -1,   206,    -1,   207,    -1,   212,    -1,   213,    -1,   214,
      -1,   215,    -1,   179,    -1,   220,    -1,   219,    -1,   221,
      -1,   180,    -1,     8,    -1,   317,    -1,     7,   137,     8,
      -1,    34,    89,    -1,    34,    89,   131,   182,   132,    -1,
     183,    -1,   182,   136,   183,    -1,     3,   133,    17,   134,
      -1,     3,   133,   326,   134,    -1,    36,    -1,   186,    -1,
     187,    -1,    35,    -1,    56,    -1,    57,    -1,    58,    -1,
      67,    -1,    75,    -1,    83,    -1,    93,    -1,    91,    -1,
      99,    -1,   100,    -1,   101,    -1,    38,    89,    -1,    39,
     131,   189,   132,    -1,   190,    -1,   190,   136,   191,    -1,
     190,   136,   191,   136,   190,    -1,   192,    -1,   190,   136,
     192,    -1,   137,   137,   137,   228,    -1,     3,   176,    -1,
     176,    -1,    47,    80,    -1,    49,   131,   195,   132,    -1,
     196,    -1,   196,   136,   137,   137,   137,    -1,   196,   136,
     137,   137,   137,   136,   196,    -1,   197,    -1,   196,   136,
     197,    -1,     3,    -1,   389,    -1,    53,    -1,    29,    -1,
      29,    45,    37,     3,    -1,    64,    78,   267,    -1,    65,
      -1,    65,   131,   388,   132,    -1,    74,    -1,   267,   137,
     299,    -1,    76,    60,    -1,    77,    89,    -1,    85,    -1,
      86,   131,   208,   132,    -1,    86,   131,   132,    -1,    86,
     131,   191,   132,    -1,   209,    -1,   209,   136,   191,    -1,
     209,   136,   191,   136,   209,    -1,   191,   136,   209,    -1,
     210,    -1,   209,   136,   210,    -1,   192,    -1,   192,    79,
      -1,    -1,   192,    44,   211,   324,    -1,    42,    78,   176,
      -1,    86,    78,   176,    -1,    87,   131,   208,   132,    -1,
      87,   131,   132,    -1,    87,    78,   176,    -1,   216,   176,
      -1,   216,    61,   176,    -1,   216,    51,   176,    -1,   138,
     218,   217,   139,    -1,    17,    -1,   326,    -1,    98,    -1,
      30,    -1,    84,    -1,    -1,     3,   140,   176,    -1,   102,
      -1,   103,    -1,   130,    -1,   222,   137,   299,    -1,   293,
      -1,   223,    -1,   296,    -1,   298,    -1,   293,   319,    -1,
     176,   226,    -1,   225,    -1,    87,   226,    78,   176,    -1,
      87,   247,    78,   176,    -1,    86,   226,    78,   176,    -1,
      86,   247,    78,   176,    -1,   133,   227,   228,   134,    -1,
     230,    -1,   253,    -1,   141,   229,    -1,    -1,   390,    -1,
     326,    -1,   176,   142,   324,    -1,   231,    -1,   231,   136,
     137,   137,   137,    -1,   137,   137,   137,   136,   231,    -1,
     231,   136,   137,   137,   137,   231,    -1,   232,    -1,    28,
     235,    -1,   233,    -1,   232,   236,   233,    -1,   234,    -1,
     233,   237,   234,    -1,   238,    -1,   238,   235,    -1,    50,
     238,    -1,   143,    -1,    96,    -1,   144,    -1,    66,    -1,
     239,    -1,   304,    -1,   133,   231,   134,    -1,   324,    -1,
     246,    -1,   240,    -1,   245,    -1,   247,    -1,   248,    -1,
     241,   137,   137,   242,    -1,   243,    -1,   243,   140,    -1,
     244,    -1,   140,   244,    -1,   324,    -1,    70,    -1,   324,
      -1,    69,    -1,    55,   226,    -1,    63,   176,    -1,   224,
      -1,   177,    -1,   179,    -1,   220,    -1,    88,   226,    -1,
     105,    41,   226,    -1,   105,    42,   249,    -1,   131,   250,
     132,    -1,   131,   137,   137,   137,   136,   250,   132,    -1,
     251,    -1,   251,   136,   250,    -1,     3,   252,    -1,     3,
     226,   252,    -1,    82,    -1,    26,    -1,    79,    -1,    -1,
     254,    -1,   258,    -1,    43,    37,   131,   255,   132,    -1,
      -1,   256,    -1,   257,   136,   256,    -1,   257,    -1,   316,
     142,   321,    -1,   321,    -1,   302,    -1,   259,    -1,   131,
     296,   132,   131,   260,   132,    -1,   260,   136,   261,    -1,
     261,    -1,   145,   262,    -1,   145,   137,   262,    -1,   262,
     137,     3,    -1,     3,    -1,     9,    31,   266,    -1,    15,
     267,    31,   301,    -1,    16,   267,    31,   302,    -1,   267,
      -1,   270,    -1,   295,    -1,   268,    -1,     9,    -1,   269,
      -1,     7,   137,     9,    -1,    59,    -1,    27,    -1,    40,
     131,   271,   132,   286,    -1,   271,   136,   272,    -1,   272,
      -1,   273,    -1,   275,    -1,   278,    -1,   279,    -1,   281,
      -1,   282,    -1,   284,    -1,    10,   274,    -1,    79,    -1,
      44,   176,    -1,    -1,    11,   176,   276,   277,    -1,    97,
      -1,    -1,    79,    -1,    44,   324,    -1,    -1,    11,   299,
     277,    -1,    12,   176,   280,    -1,    79,    -1,    44,   355,
      -1,    -1,    12,   299,   280,    -1,    13,   267,   283,    -1,
      79,    -1,    44,   301,    -1,    -1,    14,   267,   285,    -1,
      79,    -1,    44,   302,    -1,    -1,   105,    90,   287,    -1,
      -1,   131,   288,   132,    -1,   131,   132,    -1,   288,   289,
      -1,   289,    -1,   291,    -1,   290,    -1,   138,   288,   139,
      -1,   292,    -1,   300,    -1,    34,    -1,    36,    -1,    38,
      -1,    39,    -1,    47,    -1,    48,    -1,    49,    -1,    53,
      -1,    54,    -1,    64,    -1,    17,    -1,    66,    -1,    71,
      -1,    74,    -1,    76,    -1,    77,    -1,    81,    -1,    85,
      -1,    86,    -1,    87,    -1,    94,    -1,    96,    -1,   136,
      -1,   294,    -1,    15,    -1,     7,   137,    15,    -1,   267,
     319,    -1,   297,    -1,    16,    -1,     7,   137,    16,    -1,
     296,   319,    -1,   299,   137,   300,    -1,   300,    -1,    10,
      -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,   293,
      -1,   303,    -1,   223,    -1,   131,   231,   132,    -1,   222,
     137,   299,    -1,   301,    -1,   296,    -1,   298,    -1,   306,
      -1,   308,    -1,   309,    -1,   310,    -1,   311,    -1,   312,
      -1,    -1,     8,   313,   307,    31,   176,    -1,     3,   313,
     176,    31,   324,    -1,     8,   313,   176,    31,   355,    -1,
       9,   313,    31,   266,    -1,    15,   313,   267,    31,   301,
      -1,    16,   313,   267,    31,   302,    -1,   131,   314,   132,
      -1,   314,   136,   315,    -1,   315,    -1,   316,   142,   386,
      -1,   386,    -1,   176,    -1,   267,    -1,   318,   319,    -1,
     180,    -1,     8,    -1,   131,   320,   132,    -1,   320,   136,
     321,    -1,   321,    -1,   176,    -1,   324,    -1,   355,    -1,
     267,    -1,    -1,     3,   176,   323,    31,   324,    -1,   325,
      -1,   352,    -1,   334,    -1,   336,    -1,   337,    -1,   344,
      -1,   390,    -1,   345,    -1,   328,    -1,   333,    -1,   346,
      -1,   349,    -1,   327,    -1,     3,    -1,   353,    -1,     7,
     137,     3,    -1,    -1,    25,   329,   330,   132,    -1,   331,
      -1,   331,   330,    -1,     5,    -1,    17,    -1,     5,   133,
     332,   134,    -1,    17,    -1,   327,    -1,     5,    -1,    19,
      -1,    21,    -1,    20,    -1,    22,    -1,    24,   335,   132,
      -1,    24,   132,    -1,     4,    -1,   335,   136,     4,    -1,
      94,    -1,    54,    -1,   338,    -1,    18,    -1,   339,    -1,
     342,    -1,   343,    -1,    23,   340,   132,    -1,   341,    -1,
     340,   136,   341,    -1,    18,    -1,   326,    -1,    23,    17,
     136,    17,   136,    17,   136,    17,   132,    -1,    23,    17,
     136,    17,   132,    -1,     3,   142,   324,    -1,    73,    -1,
     347,    -1,   348,    -1,   146,    -1,    81,    -1,    71,    -1,
     131,   350,   132,    -1,   131,   132,    -1,   351,    -1,   350,
     136,   351,    -1,     3,   324,    -1,   326,    -1,   354,   319,
      -1,   327,    -1,     3,    -1,   131,   230,   132,    -1,     8,
      68,    31,   357,    -1,    -1,    33,   358,   359,    48,    -1,
       8,    -1,     8,   137,     8,    -1,   360,   361,    -1,    95,
      72,    31,   362,    -1,   104,    72,    31,   362,    -1,   363,
      -1,   363,   143,   362,    -1,   364,    -1,   364,   363,    -1,
     365,    -1,   366,    -1,    18,    -1,     8,    -1,     8,    31,
      -1,   106,    -1,   107,    -1,   108,    -1,   109,    -1,   110,
      -1,   110,   133,    95,     8,   134,    -1,   111,   133,   176,
     134,    -1,   111,   133,     3,   176,   134,    -1,   111,   133,
     104,   176,   134,    -1,   140,   367,   147,    -1,   368,    -1,
     367,   368,    -1,   369,    -1,   370,    -1,     8,    31,   176,
      -1,     3,   176,    31,   324,    -1,    -1,    -1,     3,   112,
     372,    90,   176,   114,   374,   115,   375,   376,   377,   378,
     381,   373,    31,   324,    -1,   116,    -1,   117,    -1,   118,
      -1,   119,    -1,   120,    -1,   121,    -1,   122,    -1,   123,
      -1,   124,    18,    -1,    -1,   125,    18,    -1,    -1,   126,
     131,   379,   132,    -1,    -1,   380,    -1,   379,   136,   380,
      -1,     3,    -1,     8,    -1,   127,   131,   324,   132,    -1,
      -1,    -1,     3,   113,   383,   128,   324,   384,   376,   377,
      31,   324,    -1,   129,   131,   385,   132,    -1,    -1,   324,
      -1,   385,   136,   324,    -1,     8,    -1,     3,    -1,     9,
      -1,    15,    -1,    16,    -1,   386,   131,   132,    -1,   389,
      -1,   388,   136,   389,    -1,     3,   133,   390,   134,    -1,
       3,   133,   326,   134,    -1,    17,    -1,   148,    17,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   427,   427,   426,   435,   440,   446,   451,   459,   460,
     466,   470,   480,   484,   488,   493,   502,   503,   508,   509,
     514,   519,   526,   527,   532,   533,   537,   538,   544,   543,
     564,   563,   577,   578,   580,   586,   590,   599,   604,   613,
     617,   623,   624,   629,   630,   631,   632,   633,   634,   635,
     637,   638,   639,   645,   644,   664,   673,   674,   675,   680,
     681,   682,   683,   684,   685,   686,   687,   688,   690,   691,
     692,   693,   694,   695,   696,   697,   698,   699,   700,   705,
     706,   707,   708,   716,   720,   726,   732,   741,   745,   753,
     758,   765,   769,   778,   786,   787,   791,   795,   799,   803,
     807,   811,   815,   819,   823,   827,   831,   835,   843,   851,
     858,   862,   866,   873,   878,   886,   891,   896,   908,   916,
     923,   927,   931,   938,   943,   954,   958,   963,   971,   975,
     983,   988,   992,  1000,  1008,  1016,  1023,  1031,  1039,  1043,
    1047,  1054,  1058,  1062,  1066,  1073,  1078,  1085,  1086,  1091,
    1090,  1099,  1107,  1115,  1119,  1127,  1135,  1140,  1145,  1153,
    1161,  1162,  1172,  1176,  1180,  1185,  1192,  1200,  1204,  1208,
    1216,  1227,  1228,  1229,  1231,  1236,  1244,  1248,  1252,  1256,
    1260,  1264,  1271,  1278,  1279,  1284,  1289,  1296,  1300,  1301,
    1311,  1315,  1319,  1323,  1331,  1332,  1341,  1346,  1353,  1358,
    1365,  1366,  1373,  1380,  1381,  1385,  1386,  1390,  1391,  1392,
    1400,  1404,  1408,  1409,  1410,  1412,  1416,  1423,  1424,  1428,
    1429,  1436,  1437,  1444,  1445,  1452,  1459,  1466,  1467,  1468,
    1469,  1474,  1482,  1486,  1493,  1497,  1504,  1509,  1517,  1521,
    1528,  1532,  1536,  1541,  1548,  1549,  1553,  1561,  1564,  1571,
    1576,  1584,  1588,  1593,  1594,  1598,  1603,  1604,  1608,  1609,
    1613,  1614,  1621,  1626,  1631,  1637,  1639,  1640,  1644,  1645,
    1646,  1651,  1658,  1662,  1670,  1674,  1675,  1679,  1680,  1681,
    1682,  1683,  1684,  1685,  1689,  1694,  1695,  1696,  1700,  1705,
    1706,  1710,  1711,  1712,  1716,  1721,  1726,  1727,  1728,  1732,
    1737,  1742,  1743,  1744,  1748,  1753,  1754,  1755,  1759,  1760,
    1764,  1765,  1769,  1770,  1774,  1775,  1779,  1783,  1784,  1789,
    1790,  1791,  1792,  1793,  1794,  1795,  1796,  1797,  1798,  1799,
    1801,  1802,  1803,  1804,  1805,  1806,  1807,  1808,  1809,  1810,
    1811,  1812,  1817,  1818,  1823,  1829,  1835,  1837,  1842,  1848,
    1853,  1857,  1862,  1863,  1864,  1865,  1866,  1871,  1877,  1879,
    1929,  1935,  1940,  1942,  1947,  1962,  1963,  1964,  1965,  1966,
    1967,  1972,  1971,  1985,  1990,  1995,  2000,  2005,  2010,  2017,
    2022,  2030,  2034,  2038,  2040,  2046,  2053,  2054,  2059,  2066,
    2071,  2079,  2080,  2082,  2084,  2099,  2098,  2116,  2117,  2122,
    2123,  2124,  2125,  2130,  2134,  2138,  2139,  2140,  2141,  2152,
    2156,  2160,  2165,  2175,  2174,  2193,  2198,  2206,  2207,  2213,
    2221,  2227,  2228,  2233,  2237,  2244,  2248,  2252,  2256,  2264,
    2268,  2277,  2281,  2289,  2296,  2300,  2301,  2302,  2306,  2313,
    2318,  2325,  2326,  2333,  2342,  2352,  2361,  2369,  2370,  2374,
    2384,  2388,  2396,  2400,  2407,  2412,  2419,  2461,  2476,  2481,
    2482,  2490,  2497,  2505,  2504,  2512,  2514,  2519,  2523,  2527,
    2549,  2550,  2554,  2555,  2559,  2560,  2564,  2566,  2568,  2570,
    2571,  2572,  2573,  2574,  2575,  2576,  2577,  2578,  2582,  2586,
    2587,  2591,  2592,  2596,  2601,  2610,  2620,  2609,  2632,  2636,
    2640,  2644,  2651,  2655,  2659,  2663,  2670,  2675,  2681,  2686,
    2692,  2697,  2703,  2708,  2715,  2716,  2720,  2725,  2732,  2731,
    2749,  2754,  2760,  2765,  2798,  2799,  2800,  2801,  2802,  2806,
    2814,  2819,  2826,  2830,  2839,  2840
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "BIT_IDENTIFIER",
  "OID_IDENTIFIER", "IMPORT_IDENTIFIER", "MODULEREFERENCE",
  "TYPEREFERENCE", "OBJECTCLASSREFERENCE", "TYPEFIELDREFERENCE",
  "VALUEFIELDREFERENCE", "VALUESETFIELDREFERENCE", "OBJECTFIELDREFERENCE",
  "OBJECTSETFIELDREFERENCE", "OBJECTREFERENCE", "OBJECTSETREFERENCE",
  "INTEGER", "CSTRING", "OS_BSTRING", "BS_BSTRING", "OS_HSTRING",
  "BS_HSTRING", "STRING_BRACE", "BITSTRING_BRACE", "OID_BRACE", "ABSENT",
  "ABSTRACT_SYNTAX", "ALL", "ANY", "APPLICATION", "ASSIGNMENT",
  "AUTOMATIC", "BEGIN_t", "BIT", "BMPString", "BOOLEAN_t", "BY",
  "CHARACTER", "CHOICE", "CLASS", "COMPONENT", "COMPONENTS", "CONSTRAINED",
  "DEFAULT", "DEFINED", "DEFINITIONS", "EMBEDDED", "END", "ENUMERATED",
  "EXCEPT", "EXPLICIT", "EXPORTS", "EXTERNAL", "FALSE_t", "FROM",
  "GeneralString", "GraphicString", "IA5String", "TYPE_IDENTIFIER",
  "IDENTIFIER_t", "IMPLICIT", "IMPORTS", "INCLUDES", "INSTANCE",
  "INTEGER_t", "INTERSECTION", "ISO646String", "MACRO", "MAX_t", "MIN_t",
  "MINUS_INFINITY", "NOTATION", "NULL_VALUE", "NULL_TYPE", "NumericString",
  "OBJECT", "OCTET", "OF_t", "OPTIONAL_t", "PDV", "PLUS_INFINITY",
  "PRESENT", "PrintableString", "PRIVATE", "REAL", "SEQUENCE", "SET",
  "SIZE_t", "STRING", "SYNTAX", "T61String", "TAGS", "TeletexString",
  "TRUE_t", "TYPE_t", "UNION", "UNIQUE", "UNIVERSAL", "UniversalString",
  "VideotexString", "VisibleString", "GeneralizedTime", "UTCTime", "VALUE",
  "WITH", "string_t", "identifier_t", "number_t", "empty_t", "type_t",
  "value_t", "OBJECT_TYPE", "TRAP_TYPE", "ACCESS", "STATUS", "read_only_t",
  "read_write_t", "write_only_t", "not_accessible_t", "mandatory_t",
  "optional_t", "obsolete_t", "deprecated_t", "DESCRIPTION_t",
  "REFERENCE_t", "INDEX_t", "DEFVAL_t", "ENTERPRISE", "VARIABLES",
  "ObjectDescriptor_t", "'{'", "'}'", "'('", "')'", "';'", "','", "'.'",
  "'['", "']'", "'<'", "'!'", "':'", "'|'", "'^'", "'@'", "'0'", "'>'",
  "'-'", "$accept", "ModuleDefinition", "$@1", "DefinitiveIdentifier",
  "DefinitiveObjIdComponentList", "DefinitiveObjIdComponent",
  "DefinitiveNameAndNumberForm", "TagDefault", "ModuleBody", "Exports",
  "SymbolsExported", "Imports", "SymbolsImported", "SymbolsFromModuleList",
  "SymbolsFromModule", "$@2", "GlobalModuleReference", "$@3",
  "AssignedIdentifier", "DefinedValue_Import", "SymbolList", "Symbol",
  "AssignmentList", "Assignment", "ValueSetTypeAssignment", "$@4",
  "TypeAssignment", "Type", "BuiltinType", "ReferencedType", "DefinedType",
  "ExternalTypeReference", "BitStringType", "NamedBitList", "NamedBit",
  "BooleanType", "CharacterStringType", "RestrictedCharacterStringType",
  "UnrestrictedCharacterStringType", "ChoiceType", "AlternativeTypeLists",
  "AlternativeTypeList", "ExtensionAndException", "NamedType",
  "EmbeddedPDVType", "EnumeratedType", "Enumerations", "Enumeration",
  "EnumerationItem", "ExternalType", "AnyType", "InstanceOfType",
  "IntegerType", "NullType", "ObjectClassFieldType",
  "ObjectIdentifierType", "OctetStringType", "RealType", "SequenceType",
  "ComponentTypeLists", "ComponentTypeList", "ComponentType", "$@5",
  "SequenceOfType", "SetType", "SetOfType", "TaggedType", "Tag",
  "ClassNumber", "Class", "SelectionType", "UsefulType", "TypeFromObject",
  "ReferencedObjects", "ParameterizedObject", "ConstrainedType",
  "TypeWithConstraint", "Constraint", "ConstraintSpec", "ExceptionSpec",
  "ExceptionIdentification", "ElementSetSpecs", "ElementSetSpec", "Unions",
  "Intersections", "IntersectionElements", "Exclusions", "UnionMark",
  "IntersectionMark", "Elements", "SubtypeElements", "ValueRange",
  "LowerEndpoint", "UpperEndpoint", "LowerEndValue", "UpperEndValue",
  "PermittedAlphabet", "ContainedSubtype", "SizeConstraint",
  "InnerTypeConstraints", "MultipleTypeConstraints", "TypeConstraints",
  "NamedConstraint", "PresenceConstraint", "GeneralConstraint",
  "UserDefinedConstraint", "UserDefinedConstraintParameterList",
  "UserDefinedConstraintParameters", "UserDefinedConstraintParameter",
  "TableConstraint", "ComponentRelationConstraint", "AtNotations",
  "AtNotation", "ComponentIdList", "ObjectClassAssignment",
  "ObjectAssignment", "ObjectSetAssignment", "ObjectClass",
  "DefinedObjectClass", "ExternalObjectClassReference",
  "UsefulObjectClassReference", "ObjectClassDefn", "FieldSpecs",
  "FieldSpec", "TypeFieldSpec", "TypeOptionalitySpec",
  "FixedTypeValueFieldSpec", "Unique", "ValueOptionalitySpec",
  "VariableTypeValueFieldSpec", "FixedTypeValueSetFieldSpec",
  "ValueSetOptionalitySpec", "VariableTypeValueSetFieldSpec",
  "ObjectFieldSpec", "ObjectOptionalitySpec", "ObjectSetFieldSpec",
  "ObjectSetOptionalitySpec", "WithSyntaxSpec", "SyntaxList",
  "TokenOrGroupSpecs", "TokenOrGroupSpec", "OptionalGroup",
  "RequiredToken", "Literal", "DefinedObject", "ExternalObjectReference",
  "ParameterizedObjectClass", "DefinedObjectSet",
  "ExternalObjectSetReference", "ParameterizedObjectSet", "FieldName",
  "PrimitiveFieldName", "Object", "ObjectSet", "ObjectFromObject",
  "ObjectSetElements", "ParameterizedAssignment",
  "ParameterizedTypeAssignment", "$@6", "ParameterizedValueAssignment",
  "ParameterizedValueSetTypeAssignment",
  "ParameterizedObjectClassAssignment", "ParameterizedObjectAssignment",
  "ParameterizedObjectSetAssignment", "ParameterList", "Parameters",
  "Parameter", "Governor", "ParameterizedType", "SimpleDefinedType",
  "ActualParameterList", "ActualParameters", "ActualParameter",
  "ValueAssignment", "$@7", "Value", "BuiltinValue", "DefinedValue",
  "ExternalValueReference", "ObjectIdentifierValue", "$@8",
  "ObjIdComponentList", "ObjIdComponent", "NumberForm", "OctetStringValue",
  "BitStringValue", "BitIdentifierList", "BooleanValue",
  "CharacterStringValue", "RestrictedCharacterStringValue",
  "CharacterStringList", "CharSyms", "CharsDefn", "Quadruple", "Tuple",
  "ChoiceValue", "NullValue", "RealValue", "NumericRealValue",
  "SpecialRealValue", "SequenceValue", "ComponentValueList", "NamedValue",
  "ReferencedValue", "ParameterizedValue", "SimpleDefinedValue",
  "ValueSet", "MacroDefinition", "MacroSubstance", "$@9", "MacroBody",
  "TypeProduction", "ValueProduction", "MacroAlternativeList",
  "MacroAlternative", "SymbolElement", "SymbolDefn", "EmbeddedDefinitions",
  "EmbeddedDefinitionList", "EmbeddedDefinition", "LocalTypeAssignment",
  "LocalValueAssignment", "ObjectTypeDefinition", "$@10", "$@11",
  "ObjectTypeAccess", "ObjectTypeStatus", "MibDescrPart", "MibReferPart",
  "MibIndexPart", "MibIndexTypes", "MibIndexType", "MibDefValPart",
  "TrapTypeDefinition", "$@12", "MibVarPart", "MibVarTypes", "Reference",
  "ParameterizedReference", "NamedNumberList", "NamedNumber",
  "SignedNumber", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   123,   125,    40,    41,    59,    44,    46,    91,    93,
      60,    33,    58,   124,    94,    64,    48,    62,    45
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   149,   151,   150,   152,   152,   153,   153,   154,   154,
     154,   155,   156,   156,   156,   156,   157,   157,   158,   158,
     159,   159,   160,   160,   161,   161,   162,   162,   164,   163,
     166,   165,   167,   167,   167,   168,   168,   169,   169,   170,
     170,   171,   171,   172,   172,   172,   172,   172,   172,   172,
     172,   172,   172,   174,   173,   175,   176,   176,   176,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   177,
     177,   177,   177,   177,   177,   177,   177,   177,   177,   178,
     178,   178,   178,   179,   179,   179,   180,   181,   181,   182,
     182,   183,   183,   184,   185,   185,   186,   186,   186,   186,
     186,   186,   186,   186,   186,   186,   186,   186,   187,   188,
     189,   189,   189,   190,   190,   191,   192,   192,   193,   194,
     195,   195,   195,   196,   196,   197,   197,   198,   199,   199,
     200,   201,   201,   202,   203,   204,   205,   206,   207,   207,
     207,   208,   208,   208,   208,   209,   209,   210,   210,   211,
     210,   210,   212,   213,   213,   214,   215,   215,   215,   216,
     217,   217,   218,   218,   218,   218,   219,   220,   220,   220,
     221,   222,   222,   222,   222,   223,   224,   224,   225,   225,
     225,   225,   226,   227,   227,   228,   228,   229,   229,   229,
     230,   230,   230,   230,   231,   231,   232,   232,   233,   233,
     234,   234,   235,   236,   236,   237,   237,   238,   238,   238,
     239,   239,   239,   239,   239,   239,   240,   241,   241,   242,
     242,   243,   243,   244,   244,   245,   246,   246,   246,   246,
     246,   247,   248,   248,   249,   249,   250,   250,   251,   251,
     252,   252,   252,   252,   253,   253,   254,   255,   255,   256,
     256,   257,   257,   258,   258,   259,   260,   260,   261,   261,
     262,   262,   263,   264,   265,   266,   266,   266,   267,   267,
     267,   268,   269,   269,   270,   271,   271,   272,   272,   272,
     272,   272,   272,   272,   273,   274,   274,   274,   275,   276,
     276,   277,   277,   277,   278,   279,   280,   280,   280,   281,
     282,   283,   283,   283,   284,   285,   285,   285,   286,   286,
     287,   287,   288,   288,   289,   289,   290,   291,   291,   292,
     292,   292,   292,   292,   292,   292,   292,   292,   292,   292,
     292,   292,   292,   292,   292,   292,   292,   292,   292,   292,
     292,   292,   293,   293,   294,   295,   296,   296,   297,   298,
     299,   299,   300,   300,   300,   300,   300,   301,   301,   301,
     302,   303,   304,   304,   304,   305,   305,   305,   305,   305,
     305,   307,   306,   308,   309,   310,   311,   312,   313,   314,
     314,   315,   315,   316,   316,   317,   318,   318,   319,   320,
     320,   321,   321,   321,   321,   323,   322,   324,   324,   325,
     325,   325,   325,   325,   325,   325,   325,   325,   325,   326,
     326,   326,   327,   329,   328,   330,   330,   331,   331,   331,
     332,   332,   332,   333,   333,   334,   334,   334,   334,   335,
     335,   336,   336,   337,   338,   338,   338,   338,   339,   340,
     340,   341,   341,   342,   343,   344,   345,   346,   346,   347,
     348,   348,   349,   349,   350,   350,   351,   352,   353,   354,
     354,   355,   356,   358,   357,   357,   357,   359,   360,   361,
     362,   362,   363,   363,   364,   364,   365,   365,   365,   365,
     365,   365,   365,   365,   365,   365,   365,   365,   366,   367,
     367,   368,   368,   369,   370,   372,   373,   371,   374,   374,
     374,   374,   375,   375,   375,   375,   376,   376,   377,   377,
     378,   378,   379,   379,   380,   380,   381,   381,   383,   382,
     384,   384,   385,   385,   386,   386,   386,   386,   386,   387,
     388,   388,   389,   389,   390,   390
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     9,     3,     0,     1,     2,     1,     1,
       1,     4,     2,     2,     2,     0,     3,     0,     3,     0,
       1,     0,     3,     0,     1,     0,     1,     2,     0,     4,
       0,     3,     1,     1,     0,     1,     1,     1,     3,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     5,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     2,     5,     1,
       3,     4,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     4,
       1,     3,     5,     1,     3,     4,     2,     1,     2,     4,
       1,     5,     7,     1,     3,     1,     1,     1,     1,     4,
       3,     1,     4,     1,     3,     2,     2,     1,     4,     3,
       4,     1,     3,     5,     3,     1,     3,     1,     2,     0,
       4,     3,     3,     4,     3,     3,     2,     3,     3,     4,
       1,     1,     1,     1,     1,     0,     3,     1,     1,     1,
       3,     1,     1,     1,     1,     2,     2,     1,     4,     4,
       4,     4,     4,     1,     1,     2,     0,     1,     1,     3,
       1,     5,     5,     6,     1,     2,     1,     3,     1,     3,
       1,     2,     2,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     4,     1,     2,     1,
       2,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       1,     2,     3,     3,     3,     7,     1,     3,     2,     3,
       1,     1,     1,     0,     1,     1,     5,     0,     1,     3,
       1,     3,     1,     1,     1,     6,     3,     1,     2,     3,
       3,     1,     3,     4,     4,     1,     1,     1,     1,     1,
       1,     3,     1,     1,     5,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     1,     2,     0,     4,     1,
       0,     1,     2,     0,     3,     3,     1,     2,     0,     3,
       3,     1,     2,     0,     3,     1,     2,     0,     3,     0,
       3,     2,     2,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     3,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     5,     5,     5,     4,     5,     5,     3,     3,
       1,     3,     1,     1,     1,     2,     1,     1,     3,     3,
       1,     1,     1,     1,     1,     0,     5,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     0,     4,     1,     2,     1,     1,     4,
       1,     1,     1,     1,     1,     1,     1,     3,     2,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     9,     5,     3,     1,     1,     1,     1,
       1,     1,     3,     2,     1,     3,     2,     1,     2,     1,
       1,     3,     4,     0,     4,     1,     3,     2,     4,     4,
       1,     3,     1,     2,     1,     1,     1,     1,     2,     1,
       1,     1,     1,     1,     5,     4,     5,     5,     3,     1,
       2,     1,     1,     3,     4,     0,     0,    16,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     0,     2,     0,
       4,     0,     1,     3,     1,     1,     4,     0,     0,    10,
       4,     0,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     3,     4,     4,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     5,     0,     0,     0,     1,     8,     9,     0,     6,
      10,    15,     0,     4,     7,     0,     0,     0,     0,     0,
      14,    12,    13,     0,    11,     2,    19,    21,     0,    23,
     525,   524,   526,   527,   528,     0,    20,    37,    39,    40,
       3,    25,     0,    18,     0,     0,     0,    24,    26,     0,
       0,     0,     0,     0,     0,    16,    41,    45,    43,    46,
      47,    48,    49,   365,   366,   367,   368,   369,   370,    44,
      50,    51,    52,    38,   529,    22,    27,    28,     0,     0,
      84,   269,   343,   347,   273,   128,     0,    96,    93,     0,
       0,     0,     0,   127,    97,    98,    99,   272,     0,   131,
     100,   133,   101,     0,     0,   102,   137,     0,     0,   104,
     103,   105,   106,   107,   167,   168,   495,   518,   169,     0,
     165,   395,    58,    57,    79,    83,    59,    60,    61,    94,
      95,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,     0,    81,
      80,    82,     0,   172,    56,   177,     0,   268,   270,   171,
     342,   173,   346,   174,     0,    85,     0,     0,     0,    53,
     371,     0,     0,     0,     0,     0,     0,     0,    42,     0,
       0,     0,     0,    87,   108,     0,   118,     0,     0,     0,
     135,   136,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   525,    84,   269,   343,   347,   383,
     384,     0,   380,     0,   382,   163,   164,   162,     0,   176,
       0,     0,     0,   156,     0,     0,     0,   175,   349,     0,
     385,    55,     0,     0,     0,     0,     0,   262,   265,   266,
     267,     0,     0,     0,     0,     0,     0,    30,    29,   166,
      86,   271,   344,   348,     0,     0,     0,   117,     0,   110,
     113,   125,     0,   120,   123,   126,   130,     0,     0,   530,
     152,   231,     0,   139,     0,     0,   147,     0,   141,   145,
     410,     0,   534,   434,   423,   425,   424,   426,     0,     0,
     413,     0,     0,   432,     0,     0,   222,   451,   446,   450,
     431,     0,     0,     0,     0,   449,     0,     0,   228,   229,
     230,     0,   359,   227,   186,   183,   190,   194,   196,   198,
     200,   207,   212,     0,   217,   213,   211,   214,   215,   184,
     244,   245,   254,   357,   363,   364,   362,   253,   358,   208,
     210,   397,   457,   409,   405,   406,   399,   400,   401,   433,
     435,   436,   437,   402,   404,   407,   447,   448,   408,   398,
     411,     0,   403,     0,     0,   155,   154,     0,     0,     0,
       0,     0,     0,   378,     0,     0,   410,     0,   160,     0,
     161,     0,   158,   157,   352,   353,   354,   355,   356,   170,
     351,   134,     0,   391,   394,     0,   390,   392,   393,     0,
     465,   463,   462,     0,     0,     0,     0,   345,   375,     0,
       0,   263,     0,     0,   264,     0,    34,   129,     0,     0,
      89,   116,   109,     0,     0,   119,     0,   132,     0,     0,
       0,   140,     0,   149,   148,   138,     0,     0,     0,     0,
     441,   442,     0,   439,   429,   428,     0,     0,     0,   195,
       0,   225,   226,     0,     0,   410,     0,   453,     0,   363,
       0,   454,     0,     0,   535,     0,     0,     0,     0,   204,
     203,     0,   206,   205,     0,   201,     0,   218,   458,   180,
     181,   153,   178,   179,     0,   410,   521,   379,   381,     0,
     159,   396,     0,     0,   388,     0,   373,     0,     0,     0,
      54,   374,   372,   287,     0,     0,     0,     0,     0,   276,
     277,   278,   279,   280,   281,   282,   283,     0,     0,   376,
     377,    36,    31,    32,    35,    33,     0,    88,     0,   111,
     114,     0,     0,     0,   124,   531,   151,   186,   144,     0,
     142,   146,   445,   412,     0,   438,     0,   427,     0,   417,
     418,     0,   415,   202,   247,   232,     0,   233,   456,     0,
     360,     0,   452,     0,   209,     0,   361,   410,     0,   185,
     188,   187,   182,     0,   197,   199,     0,     0,     0,   507,
     350,   461,   389,   466,     0,     0,     0,     0,   285,   284,
     290,   293,   298,   298,   303,   307,   309,     0,   361,     0,
       0,    90,     0,   533,   532,     0,   115,     0,   150,     0,
       0,   440,   430,     0,   414,   416,   391,     0,   248,   250,
     394,     0,   252,   243,     0,     0,   236,     0,   455,     0,
       0,     0,   224,     0,   216,   219,   223,   498,   499,   500,
     501,     0,     0,     0,   509,     0,   464,     0,   467,   286,
     289,   293,     0,   291,   294,     0,   296,   295,   299,     0,
     301,   300,     0,   305,   304,     0,   274,   275,    91,    92,
     112,   121,   143,   444,     0,   422,   420,   421,     0,   246,
       0,     0,   241,   242,   240,   243,   238,     0,   234,     0,
       0,     0,   257,   192,   189,   191,   220,     0,   522,     0,
     506,     0,     0,     0,     0,   288,   292,   297,   302,   306,
       0,     0,     0,     0,   419,   249,   251,   239,     0,   237,
     261,     0,   258,   255,     0,   193,   502,   503,   504,   505,
     507,   520,     0,   508,     0,   477,   476,   479,   480,   481,
     482,   483,     0,     0,   468,   470,   472,   474,   475,     0,
       0,   308,   122,     0,     0,   259,     0,   256,   509,   523,
     519,   478,     0,     0,     0,     0,     0,   489,   491,   492,
       0,   473,   469,   329,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   311,   341,     0,     0,   313,
     315,   314,   317,   318,     0,     0,     0,   260,   511,     0,
       0,     0,     0,     0,     0,   488,   490,   471,     0,   310,
     312,   443,   235,     0,   517,     0,     0,     0,   485,     0,
     493,   316,     0,     0,   496,   484,   486,   487,   494,   514,
     515,     0,   512,     0,     0,   510,     0,     0,     0,   513,
     516,   497
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    26,     4,     8,     9,    10,    18,    28,    29,
      35,    42,    46,    47,    48,   179,   248,   416,   522,   523,
      49,    37,    55,    56,    57,   233,    58,   307,   122,   123,
     124,   125,   126,   419,   420,   127,   128,   129,   130,   131,
     258,   259,   275,   276,   132,   133,   262,   263,   264,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   277,
     278,   279,   539,   144,   145,   146,   147,   148,   379,   218,
     149,   150,   151,   152,   153,   154,   155,   219,   314,   467,
     569,   493,   316,   317,   318,   319,   449,   471,   474,   320,
     321,   322,   323,   634,   324,   635,   325,   326,   327,   328,
     557,   625,   626,   686,   329,   330,   617,   618,   619,   331,
     332,   691,   692,   722,    59,    60,    61,   237,   156,   157,
     158,   239,   508,   509,   510,   589,   511,   651,   654,   512,
     513,   657,   514,   515,   661,   516,   664,   666,   751,   798,
     799,   800,   801,   802,   159,   160,   240,   161,   162,   163,
     389,   390,   336,   337,   338,   339,    62,    63,   235,    64,
      65,    66,    67,    68,   164,   211,   212,   213,   165,   166,
     228,   395,   622,    69,   220,   340,   341,   342,   343,   344,
     447,   551,   552,   678,   345,   346,   446,   347,   348,   349,
     350,   442,   443,   351,   352,   353,   354,   355,   356,   357,
     358,   460,   461,   359,   360,   361,   398,    70,   402,   498,
     585,   586,   648,   744,   745,   746,   747,   748,   766,   767,
     768,   769,    71,   202,   844,   641,   730,   644,   702,   824,
     841,   842,   834,    72,   203,   579,   699,    38,    39,   268,
     265,   362
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -727
static const yytype_int16 yypact[] =
{
     105,     6,   151,   115,   140,  -727,    64,  -727,    78,   115,
    -727,    27,   204,  -727,  -727,   169,   206,   213,   292,   211,
    -727,  -727,  -727,   319,  -727,  -727,    61,   288,   306,   332,
    -727,  -727,  -727,  -727,  -727,   231,  -727,   247,   261,  -727,
    -727,   288,   355,  -727,   288,   298,   302,   288,  -727,   365,
    2113,  2221,     5,    56,    56,   355,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,   323,   305,
     346,  -727,  -727,  -727,  -727,   416,   390,  -727,  -727,   397,
     370,   411,   371,  -727,  -727,  -727,  -727,  -727,   422,   372,
    -727,  -727,  -727,   447,   419,  -727,  -727,    81,   104,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  3063,
      18,   376,  -727,  -727,  -727,   374,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  2538,  -727,
    -727,  -727,   375,  -727,  -727,  -727,   378,  -727,  -727,   385,
    -727,   385,  -727,  -727,  3168,  -727,   385,  3168,   486,   376,
    3168,   334,   487,   383,   490,    47,   491,    47,  -727,   516,
    3168,   381,   488,   396,  -727,  3273,  -727,   526,    47,   527,
    -727,  -727,  3168,   376,  2326,   716,   454,   455,  3168,  2432,
     456,   457,   446,   410,   323,   -56,   122,   147,   150,   376,
     378,   176,  -727,   398,  -727,  -727,  -727,  -727,   191,  -727,
     508,  3168,  3168,   376,   388,   388,  1526,  -727,  -727,     1,
    -727,   376,    58,   510,    13,   511,   412,  -727,   385,  -727,
    -727,   334,   535,   273,   514,   417,   518,  -727,  -727,   376,
    -727,  -727,  -727,  -727,   547,   551,  1901,   376,   423,   420,
    -727,   427,   438,   435,  -727,  -727,  -727,   427,   259,  -727,
     376,  -727,   494,  -727,   436,   271,     3,   442,   439,  -727,
      76,   441,  -727,  -727,  -727,  -727,  -727,  -727,   239,    15,
    -727,   529,   539,  -727,   376,  3168,  -727,  -727,  -727,  -727,
    -727,   363,   988,  1258,   443,  -727,   564,   376,   449,   450,
     451,   448,   452,   453,   458,  -727,   459,   -35,   -39,  -727,
     529,  -727,  -727,   460,   461,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,    52,   107,   463,  -727,  -727,  -727,  -727,
     207,  -727,  -727,   462,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,   385,  -727,  3168,  3168,   376,  -727,   466,   471,  3168,
    3168,  3168,  1807,  -727,  3063,   288,   474,   469,  -727,   468,
    -727,  1807,   376,   376,  -727,  -727,  -727,  -727,  -727,   472,
    -727,   472,   852,   376,   378,   282,  -727,  -727,  -727,  1807,
     473,  -727,  -727,   477,   477,  3168,   399,  -727,  -727,   475,
     476,  -727,   273,  1258,  -727,   417,   118,  -727,   478,   285,
    -727,   376,  -727,  2748,    32,  -727,    30,  -727,   527,  3168,
     479,  -727,  2853,  -727,  -727,  -727,  2643,  1807,   369,   482,
    -727,  -727,   296,  -727,  -727,  -727,   297,   234,  1392,  -727,
     483,  -727,   376,   376,   489,   544,    23,  -727,   492,   128,
     299,  -727,   485,   497,  -727,   388,  1631,   493,   499,  -727,
    -727,  1392,  -727,  -727,  1392,  -727,   500,  -727,  -727,   376,
     376,  -727,   376,   376,   -47,   -61,   465,  -727,  -727,   584,
    -727,  -727,   388,   496,  -727,  1526,  -727,   580,   495,  1122,
    -727,  -727,   376,    16,  2008,  2008,    47,    47,   304,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,   360,   388,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,   210,  -727,   551,   503,
    -727,   507,   512,   505,  -727,  -727,   376,   458,   509,  1807,
     513,  -727,  -727,  -727,   574,  -727,   212,  -727,   588,   515,
    -727,   519,   234,  -727,  1526,  -727,    37,  -727,  -727,  1807,
    -727,   498,  -727,   619,  -727,   517,   232,   154,   167,  -727,
    -727,  -727,  -727,   520,   -39,  -727,  1718,   352,   521,   502,
    -727,  -727,  -727,  -727,   572,   575,   543,  3168,  -727,  -727,
     -40,    -6,    24,     8,    40,    84,   545,   399,   472,   522,
     524,  -727,  3273,  -727,  -727,   523,  -727,  2853,  -727,  2853,
     307,  -727,  -727,   217,  -727,  -727,   194,   531,  -727,   528,
      83,   525,  -727,    17,   532,   533,   530,   534,  -727,  1258,
    1807,   536,  -727,  1798,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,   540,  1807,   636,   546,   628,  -727,   596,  -727,   376,
    -727,    89,  1807,  -727,  -727,   477,  -727,  -727,  -727,   273,
    -727,  -727,   417,  -727,  -727,   582,  -727,  -727,  -727,  -727,
     538,   541,   509,  -727,   659,  -727,  -727,  -727,   548,  -727,
    1526,  1526,  -727,  -727,  -727,    25,  -727,   550,  -727,   675,
      38,   320,  -727,  -727,  -727,  1258,  -727,   361,  -727,   321,
    -727,   662,   650,    93,   652,  -727,  -727,  -727,  -727,  -727,
     554,  3273,   526,   552,  -727,  -727,  -727,  -727,   553,  -727,
    -727,   688,   556,  -727,   534,  -727,  -727,  -727,  -727,  -727,
     502,  -727,  1807,  -727,  1807,   663,  -727,  -727,  -727,  -727,
    -727,   562,   563,   233,  -727,   555,    93,  -727,  -727,    93,
    3446,  -727,   561,   682,   675,   556,   697,  -727,   546,  -727,
    -727,  -727,   606,  2958,  3168,   671,    34,  -727,  -727,  -727,
      93,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  3630,  3538,  -727,
    -727,  -727,  -727,  -727,   526,   571,   573,  -727,   578,   698,
    1901,  3168,   317,    19,  3168,  -727,  -727,  -727,  3368,  -727,
    -727,  -727,  -727,   576,   581,   577,   342,   356,  -727,  1807,
     376,  -727,   330,   579,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,   328,  -727,  1807,   681,  -727,   330,   583,  1807,  -727,
    -727,  -727
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -727,  -727,  -727,  -727,   707,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,   670,  -727,  -727,  -727,  -727,  -727,
     114,  -727,  -727,   665,  -727,  -727,  -727,   -50,  -142,  -727,
    -137,  -727,  -727,  -727,   190,  -727,  -727,  -727,  -727,  -727,
    -727,   119,  -170,  -183,  -727,  -727,  -727,    10,  -417,  -727,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,   549,
    -421,  -411,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -118,  -727,  -181,  -164,   -33,  -727,  -104,  -727,   189,
    -727,   558,  -287,  -727,   256,   254,   409,  -727,  -727,   294,
    -727,  -727,  -727,  -727,  -727,    97,  -727,  -727,   386,  -727,
    -727,  -644,  -727,    62,  -727,  -727,  -727,    66,  -727,  -727,
    -727,  -727,    33,    28,  -727,  -727,  -727,   537,   -48,  -727,
    -727,  -727,  -727,   159,  -727,  -727,  -727,  -727,   111,  -727,
    -727,   165,  -727,  -727,  -727,  -727,  -727,  -727,  -727,   -31,
    -726,  -727,  -727,  -727,  -149,  -727,  -727,   -25,  -727,    14,
    -203,  -458,  -233,  -238,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,  -727,  -727,   445,  -727,   393,  -524,  -727,  -727,
    -138,  -727,  -214,  -727,  -727,  -208,  -727,  -198,  -408,   366,
    -727,   216,  -727,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,   230,  -727,  -727,  -727,  -727,  -727,  -727,  -727,
    -727,  -727,   214,  -727,  -727,  -727,  -380,  -727,  -727,  -727,
    -727,  -727,  -727,  -685,    39,  -727,  -727,  -727,  -727,    22,
    -727,  -727,  -727,  -727,  -727,  -727,  -727,    54,    36,  -727,
    -727,   -51,  -727,  -727,  -727,  -727,  -727,  -102,  -727,  -727,
    -176,  -369
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -529
static const yytype_int16 yytable[] =
{
     121,   169,   260,   196,   200,   174,   176,   414,   524,   534,
     411,   538,   396,   269,   311,   458,   462,   214,   397,   444,
     380,   227,   391,   500,   501,   541,   559,   472,   230,   367,
     621,   312,   399,   261,   580,   376,   171,   764,   652,   377,
     623,   720,   765,   682,   404,   719,   333,   433,   215,   282,
     829,   682,   655,   308,   173,   532,    81,   650,   309,    15,
     587,   469,   410,   173,   772,    81,   400,   577,   655,   209,
    -460,   210,   820,   653,    84,  -387,  -524,   310,    16,   312,
    -524,   437,   434,    84,   659,   817,   195,   656,    17,   271,
     441,   401,   820,   195,   333,   588,   683,   571,   223,   684,
     407,   735,   216,   656,   683,   473,    97,   684,   470,   -17,
     806,   736,     1,    27,   229,    97,   217,   231,     6,   660,
     234,   311,   311,   238,   521,   377,   458,   244,   662,   246,
     249,   492,     7,   652,   195,   257,   119,     3,   312,   312,
     266,    36,   270,   290,   257,   492,   195,   445,   365,   257,
     195,     5,   195,   333,   333,   457,   621,   195,    73,   192,
     308,   308,   313,   663,   486,   309,   309,   533,   653,   193,
     334,   382,   383,   491,   624,   721,   393,   520,   394,   519,
     306,   815,   198,   226,   310,   310,    11,   119,   672,  -171,
     451,   496,   193,   238,   376,   227,   541,    12,   377,   737,
     738,   739,   740,   741,   742,   677,   421,  -460,   378,   335,
      13,   311,   194,   376,   195,   376,   180,   377,   437,   377,
     225,    19,   675,   478,   377,  -384,   531,   599,   312,   542,
     440,   410,   311,   743,   676,   199,   764,   195,   226,   549,
     530,   765,   376,   333,  -173,   452,   377,   558,   312,   312,
     308,   550,   535,   529,  -526,   309,   439,   440,  -526,   226,
     561,    20,   566,   333,   333,  -173,   540,   311,   570,   313,
     313,   308,   214,   488,   310,   707,   309,   459,   334,  -527,
     409,   582,  -528,  -527,   312,  -460,  -528,   397,    82,    83,
     311,    30,   803,   311,   180,   310,    31,    32,    21,   333,
     195,   591,   593,    33,    34,    22,   308,   312,   373,   630,
     312,   309,   374,   479,   480,   598,   335,   335,   311,   482,
     483,   484,   333,    23,   209,   333,   210,   195,   600,   308,
     310,   608,   308,   839,   309,   312,  -383,   309,   840,   803,
     803,   173,   693,    81,  -221,    24,   397,  -221,   441,   555,
     333,   558,    25,   310,    40,   502,   310,   308,    50,   313,
     803,    84,   309,    51,    52,  -170,    43,   334,   636,   492,
      53,    54,   543,   257,   236,   252,   253,   250,   251,   536,
     313,   310,   257,    44,   252,   253,   257,   534,   334,   250,
     251,   427,    45,    97,    41,   428,   252,   253,   384,   385,
     386,   387,   388,   431,   453,   454,   335,   432,   725,   503,
     504,   505,   506,   507,   494,   313,   568,   527,   495,   260,
      77,   528,   694,   334,   709,   636,   708,   335,   545,   547,
      74,   562,   546,   548,   698,   563,   596,    75,   313,   673,
     597,   313,   181,   674,   706,   393,   334,   394,   311,   334,
     195,   828,   723,   731,   590,   592,   724,   732,   594,   595,
     845,   182,   335,   180,   846,   312,   313,   716,   637,   638,
     639,   640,   397,   397,   334,   195,   836,  -387,   410,   183,
     333,   726,   727,   728,   729,   335,   184,   308,   335,   195,
     837,   186,   309,   197,   201,   312,   170,   172,   175,   177,
     188,   185,   187,   189,   616,  -386,   620,   190,   191,   195,
     333,   310,   224,   335,   311,   225,   226,   232,   241,   685,
     242,   243,   245,   247,   759,   254,   760,   255,   530,   261,
     267,   312,   363,   364,   369,   370,   371,   649,   372,   381,
     375,   403,   405,   406,   251,   412,   333,   485,   413,   415,
     417,   377,   257,   308,   418,   422,   423,   257,   309,   257,
     424,   282,   283,   284,   285,   286,   287,   288,   289,   290,
     425,   426,   429,   430,   435,   436,   450,   310,   438,   448,
     463,   464,   -58,   -79,   -80,   465,   -56,   543,   583,  -172,
     584,   610,   612,  -459,   578,   468,   313,   476,   293,   466,
    -174,   477,   432,   481,   334,  -460,   489,   490,   499,   492,
     497,   526,   517,   518,   554,   297,   537,   298,   544,   564,
     556,   838,   559,   646,   560,   299,   643,   572,   581,   627,
     616,   393,   620,   394,   565,   847,   573,   576,   300,   602,
     851,   603,   605,   335,   645,   607,   604,   647,   613,   609,
     665,   614,   642,   629,   700,   697,   668,   631,   669,   703,
     671,   257,   313,   679,   680,   688,   689,   681,   704,   687,
     334,   701,   710,   695,   711,   456,   713,   712,   623,   690,
     733,   734,   714,   749,   180,   750,   437,   718,   753,   754,
     305,   720,   306,   756,   761,   762,   763,   804,   770,   805,
     807,   809,   814,   821,   823,   822,   825,   832,   833,   335,
     843,   835,   848,   812,   813,   850,    14,    76,   601,   280,
     178,   670,   752,   281,    80,    81,   606,   574,   575,   475,
     696,    82,    83,   282,   283,   284,   285,   286,   287,   288,
     289,   290,   553,    84,   291,    85,   715,   717,   368,   755,
      86,    87,    88,   315,    89,    90,   667,   757,   658,   292,
     826,   827,   705,    91,   830,    92,   818,   487,   615,    93,
     293,   294,    94,    95,    96,    97,   611,   628,   408,   295,
      98,    99,   525,   100,   758,   771,   296,   297,   816,   298,
     101,   102,   103,   104,   808,   849,     0,   299,     0,   105,
       0,   106,   107,   108,   193,     0,     0,   109,     0,   110,
     300,     0,     0,     0,     0,   111,   112,   113,   114,   115,
       0,   301,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   118,   302,     0,   303,
       0,     0,     0,   304,   120,   455,     0,     0,     0,   281,
      80,    81,   305,     0,   306,     0,     0,    82,    83,   282,
     283,   284,   285,   286,   287,   288,   289,   290,     0,    84,
     291,    85,     0,     0,     0,     0,    86,    87,    88,     0,
      89,    90,     0,     0,     0,     0,     0,     0,     0,    91,
       0,    92,     0,     0,     0,    93,   293,   294,    94,    95,
      96,    97,     0,     0,     0,   295,    98,    99,     0,   100,
       0,     0,   296,   297,     0,   298,   101,   102,   103,   104,
       0,     0,     0,   299,     0,   105,     0,   106,   107,   108,
     193,     0,     0,   109,     0,   110,   300,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,   301,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,   456,   457,   303,     0,     0,     0,   304,
     120,   455,     0,     0,     0,   281,    80,    81,   305,     0,
     306,     0,     0,    82,    83,   282,   283,   284,   285,   286,
     287,   288,   289,   290,     0,    84,   291,    85,     0,     0,
       0,     0,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,   293,   294,    94,    95,    96,    97,     0,     0,
       0,   295,    98,    99,     0,   100,     0,     0,   296,   297,
       0,   298,   101,   102,   103,   104,     0,     0,     0,   299,
       0,   105,     0,   106,   107,   108,   193,     0,     0,   109,
       0,   110,   300,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,   301,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   118,   456,
     457,   303,     0,     0,     0,   280,   120,     0,     0,   281,
      80,    81,     0,     0,   305,     0,   306,    82,    83,   282,
     283,   284,   285,   286,   287,   288,   289,   290,     0,    84,
     291,    85,     0,     0,     0,     0,    86,    87,    88,     0,
      89,    90,     0,     0,     0,     0,     0,     0,     0,    91,
       0,    92,     0,     0,     0,    93,   293,   294,    94,    95,
      96,    97,     0,     0,     0,   295,    98,    99,     0,   100,
       0,     0,   296,   297,     0,   298,   101,   102,   103,   104,
       0,     0,     0,   299,     0,   105,     0,   106,   107,   108,
     193,     0,     0,   109,     0,   110,   300,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,   301,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,   456,     0,   303,     0,     0,     0,   304,
     120,   280,     0,     0,     0,   281,    80,    81,   305,     0,
     306,     0,     0,    82,    83,   282,   283,   284,   285,   286,
     287,   288,   289,   290,     0,    84,   291,    85,     0,     0,
       0,     0,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,   293,   294,    94,    95,    96,    97,     0,     0,
       0,   295,    98,    99,     0,   100,     0,     0,   296,   297,
       0,   298,   101,   102,   103,   104,     0,     0,     0,   299,
       0,   105,     0,   106,   107,   108,   193,     0,     0,   109,
       0,   110,   300,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,   301,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   118,   456,
       0,   303,     0,     0,     0,   280,   120,     0,     0,   281,
      80,    81,     0,     0,   305,     0,   306,    82,    83,   282,
     283,   284,   285,   286,   287,   288,   289,   290,     0,    84,
       0,    85,     0,     0,     0,     0,    86,    87,    88,     0,
      89,    90,     0,     0,     0,     0,     0,     0,     0,    91,
       0,    92,     0,     0,     0,    93,   293,   294,    94,    95,
      96,    97,     0,     0,     0,   295,    98,    99,     0,   100,
       0,     0,   296,   297,     0,   298,   101,   102,   103,   104,
       0,     0,     0,   299,     0,   105,     0,   106,   107,   108,
     193,     0,     0,   109,     0,   110,   300,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,   301,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   118,   456,     0,   303,     0,     0,     0,   280,
     120,     0,     0,   281,    80,    81,     0,     0,   305,     0,
     306,    82,    83,   282,   283,   284,   285,   286,   287,   288,
     289,   290,     0,    84,     0,    85,     0,     0,     0,     0,
      86,    87,    88,     0,    89,    90,     0,     0,     0,     0,
       0,     0,     0,    91,     0,    92,     0,     0,     0,    93,
     293,     0,    94,    95,    96,    97,     0,     0,     0,     0,
      98,    99,     0,   100,     0,     0,     0,   297,     0,   298,
     101,   102,   103,   104,     0,     0,     0,   299,     0,   105,
       0,   106,   107,   108,     0,     0,     0,   109,     0,   110,
     300,     0,     0,     0,     0,   111,   112,   113,   114,   115,
       0,     0,     0,     0,   567,     0,     0,     0,   281,    80,
      81,     0,     0,     0,     0,     0,    82,    83,   282,     0,
       0,     0,     0,     0,     0,     0,   118,   392,    84,     0,
      85,     0,     0,     0,   120,    86,    87,    88,     0,    89,
      90,     0,   305,     0,   306,     0,     0,     0,    91,     0,
      92,     0,     0,     0,    93,     0,     0,    94,    95,    96,
      97,     0,     0,     0,     0,    98,    99,     0,   100,     0,
       0,     0,     0,     0,     0,   101,   102,   103,   104,     0,
       0,     0,     0,     0,   105,     0,   106,   107,   108,     0,
       0,   485,   109,     0,   110,   377,     0,     0,     0,     0,
     111,   112,   113,   114,   115,   282,   283,   284,   285,   286,
     287,   288,   289,   290,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   118,     0,     0,     0,     0,     0,     0,     0,   120,
       0,     0,   293,     0,     0,     0,     0,     0,     0,   306,
       0,     0,     0,     0,     0,     0,     0,   632,     0,   297,
       0,   298,     0,     0,     0,     0,     0,     0,     0,   299,
       0,   485,     0,     0,     0,   377,     0,     0,     0,     0,
     485,     0,   300,     0,   377,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   282,   283,   284,   285,   286,   287,
     288,   289,   290,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   456,
       0,     0,   293,     0,     0,     0,     0,     0,   633,     0,
       0,   293,     0,     0,   305,     0,   306,   632,     0,   297,
       0,   298,     0,     0,     0,     0,     0,     0,   297,   299,
     298,     0,     0,     0,     0,     0,     0,     0,   299,     0,
       0,     0,   300,     0,     0,     0,     0,     0,     0,     0,
       0,   300,     0,     0,    78,     0,     0,     0,    79,    80,
      81,     0,     0,     0,     0,     0,    82,    83,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    84,   456,
      85,     0,     0,     0,     0,    86,    87,    88,   456,    89,
      90,     0,     0,     0,   305,     0,   306,     0,    91,     0,
      92,     0,     0,   305,    93,   306,     0,    94,    95,    96,
      97,     0,     0,     0,     0,    98,    99,     0,   100,     0,
       0,     0,     0,     0,     0,   101,   102,   103,   104,     0,
       0,     0,     0,     0,   105,     0,   106,   107,   108,     0,
       0,     0,   109,     0,   110,     0,     0,     0,     0,     0,
     111,   112,   113,   114,   115,     0,     0,     0,     0,     0,
       0,    78,     0,     0,     0,    79,    80,    81,   384,   385,
     386,   387,   388,    82,    83,     0,     0,     0,     0,     0,
       0,   118,     0,     0,     0,    84,     0,    85,     0,   120,
       0,   180,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,     0,     0,    94,    95,    96,    97,     0,     0,
       0,     0,    98,    99,     0,   100,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,   105,     0,   106,   107,   108,     0,     0,     0,   109,
       0,   110,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,     0,     0,     0,    78,     0,     0,     0,
      79,    80,    81,     0,     0,     0,     0,     0,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
      84,     0,    85,     0,     0,     0,   120,    86,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
      91,     0,    92,     0,     0,     0,    93,     0,     0,    94,
      95,    96,    97,     0,     0,     0,     0,    98,    99,     0,
     100,     0,     0,     0,     0,     0,     0,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   105,     0,   106,   107,
     108,     0,     0,     0,   109,     0,   110,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,     0,     0,
       0,     0,     0,     0,    78,   116,   117,     0,    79,    80,
      81,     0,     0,     0,     0,     0,    82,    83,     0,     0,
       0,     0,     0,   118,   119,     0,     0,     0,    84,     0,
      85,   120,   167,     0,     0,    86,    87,    88,     0,    89,
      90,     0,     0,     0,     0,     0,     0,     0,    91,     0,
      92,     0,     0,     0,    93,     0,     0,    94,    95,    96,
      97,     0,     0,     0,     0,    98,    99,     0,   100,   168,
       0,     0,     0,     0,     0,   101,   102,   103,   104,     0,
       0,     0,     0,     0,   105,     0,   106,   107,   108,     0,
       0,     0,   109,     0,   110,     0,     0,     0,     0,     0,
     111,   112,   113,   114,   115,     0,     0,     0,     0,   256,
       0,     0,     0,    79,    80,    81,     0,     0,     0,     0,
       0,    82,    83,     0,     0,     0,     0,     0,     0,     0,
       0,   118,   119,    84,     0,    85,     0,     0,     0,   120,
      86,    87,    88,     0,    89,    90,     0,     0,   272,     0,
       0,     0,     0,    91,     0,    92,     0,     0,     0,    93,
       0,     0,    94,    95,    96,    97,     0,     0,     0,     0,
      98,    99,     0,   100,     0,     0,     0,     0,     0,     0,
     101,   102,   103,   104,     0,     0,     0,     0,     0,   105,
       0,   106,   107,   108,     0,     0,     0,   109,     0,   110,
       0,     0,     0,     0,     0,   111,   112,   113,   114,   115,
       0,     0,     0,     0,     0,   256,     0,     0,     0,    79,
      80,    81,     0,     0,     0,     0,     0,    82,    83,     0,
       0,     0,     0,     0,     0,     0,   118,     0,   273,    84,
       0,    85,     0,   274,   120,     0,    86,    87,    88,     0,
      89,    90,     0,     0,   272,     0,     0,     0,     0,    91,
       0,    92,     0,     0,     0,    93,     0,     0,    94,    95,
      96,    97,     0,     0,     0,     0,    98,    99,     0,   100,
       0,     0,     0,     0,     0,     0,   101,   102,   103,   104,
       0,     0,     0,     0,     0,   105,     0,   106,   107,   108,
       0,     0,     0,   109,     0,   110,     0,     0,     0,     0,
       0,   111,   112,   113,   114,   115,     0,     0,     0,     0,
       0,    78,     0,     0,     0,    79,    80,    81,     0,     0,
       0,     0,     0,    82,    83,     0,     0,     0,     0,     0,
       0,     0,   118,     0,   366,    84,     0,    85,     0,   274,
     120,     0,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,   221,
       0,    93,     0,     0,    94,    95,    96,    97,     0,   222,
       0,     0,    98,    99,     0,   100,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,   105,     0,   106,   107,   108,     0,     0,     0,   109,
       0,   110,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,     0,     0,     0,   256,     0,     0,     0,
      79,    80,    81,     0,     0,     0,     0,     0,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
      84,     0,    85,     0,     0,     0,   120,    86,    87,    88,
       0,    89,    90,     0,     0,   272,     0,     0,     0,     0,
      91,     0,    92,     0,     0,     0,    93,     0,     0,    94,
      95,    96,    97,     0,     0,     0,     0,    98,    99,     0,
     100,     0,     0,     0,     0,     0,     0,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   105,     0,   106,   107,
     108,     0,     0,     0,   109,     0,   110,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,     0,     0,
       0,   256,     0,     0,     0,    79,    80,    81,     0,     0,
       0,     0,     0,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,   118,     0,    84,     0,    85,     0,     0,
     274,   120,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,     0,     0,    94,    95,    96,    97,     0,     0,
       0,     0,    98,    99,     0,   100,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,   105,     0,   106,   107,   108,     0,     0,     0,   109,
       0,   110,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,     0,     0,     0,   256,     0,     0,     0,
      79,    80,    81,     0,     0,     0,     0,     0,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
      84,     0,    85,     0,     0,   274,   120,    86,    87,    88,
       0,    89,    90,     0,     0,   272,     0,     0,     0,     0,
      91,     0,    92,     0,     0,     0,    93,     0,     0,    94,
      95,    96,    97,     0,     0,     0,     0,    98,    99,     0,
     100,     0,     0,     0,     0,     0,     0,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   105,     0,   106,   107,
     108,     0,     0,     0,   109,     0,   110,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,     0,     0,
       0,   810,     0,     0,     0,    79,    80,    81,     0,     0,
       0,     0,     0,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,   118,     0,    84,     0,    85,     0,     0,
       0,   120,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,     0,     0,    94,    95,    96,    97,     0,     0,
       0,     0,    98,    99,     0,   100,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,   105,     0,   106,   107,   108,     0,     0,     0,   109,
       0,   110,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,   811,     0,     0,     0,   204,     0,     0,     0,
      79,   205,   206,     0,     0,     0,     0,     0,   207,   208,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
      84,     0,    85,     0,     0,     0,   120,    86,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
      91,     0,    92,     0,     0,     0,    93,     0,     0,    94,
      95,    96,    97,     0,     0,     0,     0,    98,    99,     0,
     100,     0,     0,     0,     0,     0,     0,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   105,     0,   106,   107,
     108,     0,     0,     0,   109,     0,   110,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,     0,     0,
       0,    78,     0,     0,     0,    79,    80,    81,     0,     0,
       0,     0,     0,    82,    83,     0,     0,     0,     0,     0,
       0,     0,     0,   118,     0,    84,     0,    85,     0,     0,
       0,   120,    86,    87,    88,     0,    89,    90,     0,     0,
       0,     0,     0,     0,     0,    91,     0,    92,     0,     0,
       0,    93,     0,     0,    94,    95,    96,    97,     0,     0,
       0,     0,    98,    99,     0,   100,     0,     0,     0,     0,
       0,     0,   101,   102,   103,   104,     0,     0,     0,     0,
       0,   105,     0,   106,   107,   108,     0,     0,     0,   109,
       0,   110,     0,     0,     0,     0,     0,   111,   112,   113,
     114,   115,     0,     0,     0,     0,   256,     0,     0,     0,
      79,    80,    81,     0,     0,     0,     0,     0,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,   118,     0,
      84,     0,    85,     0,     0,     0,   120,    86,    87,    88,
       0,    89,    90,     0,     0,     0,     0,     0,     0,     0,
      91,     0,    92,     0,     0,     0,    93,     0,     0,    94,
      95,    96,    97,     0,     0,     0,     0,    98,    99,     0,
     100,     0,     0,     0,     0,     0,     0,   101,   102,   103,
     104,     0,     0,     0,     0,     0,   105,     0,   106,   107,
     108,     0,     0,     0,   109,     0,   110,     0,     0,     0,
       0,     0,   111,   112,   113,   114,   115,     0,   384,   385,
     386,   387,   388,     0,     0,   773,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   774,   118,   775,     0,   776,   777,     0,     0,
       0,   120,     0,     0,     0,   778,   779,   780,     0,     0,
       0,   781,   782,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   783,     0,   784,     0,     0,     0,     0,   785,
       0,     0,   786,     0,   787,   788,     0,     0,     0,   789,
       0,     0,     0,   790,   791,   792,   384,   385,   386,   387,
     388,     0,   793,   773,   794,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     774,     0,   775,     0,   776,   777,     0,     0,     0,     0,
       0,     0,     0,   778,   779,   780,     0,     0,     0,   781,
     782,     0,     0,     0,   796,     0,   797,   831,     0,     0,
     783,     0,   784,     0,     0,     0,     0,   785,     0,     0,
     786,     0,   787,   788,     0,     0,     0,   789,     0,     0,
       0,   790,   791,   792,     0,     0,     0,     0,     0,     0,
     793,     0,   794,     0,     0,     0,     0,     0,   384,   385,
     386,   387,   388,     0,     0,   773,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   774,     0,   775,     0,   776,   777,   795,     0,
       0,     0,   796,     0,   797,   778,   779,   780,     0,     0,
       0,   781,   782,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   783,     0,   784,     0,     0,     0,     0,   785,
       0,     0,   786,     0,   787,   788,     0,     0,     0,   789,
       0,     0,     0,   790,   791,   792,     0,     0,     0,     0,
       0,     0,   793,     0,   794,     0,     0,     0,     0,     0,
     384,   385,   386,   387,   388,     0,     0,   773,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   774,     0,   775,     0,   776,   777,
     819,     0,     0,     0,   796,     0,   797,   778,   779,   780,
       0,     0,     0,   781,   782,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   783,     0,   784,     0,     0,     0,
       0,   785,     0,     0,   786,     0,   787,   788,     0,     0,
       0,   789,     0,     0,     0,   790,   791,   792,     0,     0,
       0,     0,     0,     0,   793,     0,   794,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   796,     0,   797
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-727)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      50,    51,   185,   107,   108,    53,    54,   245,   416,   426,
     243,   432,   226,   189,   195,   302,   303,   119,   226,     4,
     218,   159,   225,   403,   404,   436,     3,    66,   166,   199,
     554,   195,    31,     3,   492,     3,    31,     3,    44,     7,
       3,     3,     8,    26,    31,   689,   195,    44,    30,    17,
      31,    26,    44,   195,     7,   424,     9,    97,   195,    32,
      44,    96,   243,     7,   749,     9,     8,   114,    44,   119,
     131,   119,   798,    79,    27,   131,   132,   195,    51,   243,
     136,   142,    79,    27,    44,   770,   133,    79,    61,   193,
     288,    33,   818,   133,   243,    79,    79,   466,   148,    82,
     238,     8,    84,    79,    79,   144,    59,    82,   143,    48,
     754,    18,     7,    52,   164,    59,    98,   167,     3,    79,
     170,   302,   303,   171,     6,     7,   413,   175,    44,   177,
     180,   137,    17,    44,   133,   185,   131,   131,   302,   303,
     188,    27,   192,    25,   194,   137,   133,   132,   198,   199,
     133,     0,   133,   302,   303,   132,   680,   133,    44,    78,
     302,   303,   195,    79,   372,   302,   303,   137,    79,    88,
     195,   221,   222,   381,   137,   137,   226,   415,   226,   412,
     148,   147,    78,   131,   302,   303,    46,   131,   609,   137,
     294,   399,    88,   241,     3,   333,   607,   133,     7,   106,
     107,   108,   109,   110,   111,   613,   256,   131,    17,   195,
     132,   392,   131,     3,   133,     3,   140,     7,   142,     7,
     137,    17,     5,   361,     7,   142,   424,    17,   392,   437,
      18,   412,   413,   140,    17,   131,     3,   133,   131,     5,
     423,     8,     3,   392,   137,   295,     7,   455,   412,   413,
     392,    17,   428,   423,   132,   392,    17,    18,   136,   131,
     132,    92,   465,   412,   413,   137,   436,   448,   466,   302,
     303,   413,   374,   375,   392,   655,   413,   302,   303,   132,
       7,   495,   132,   136,   448,   131,   136,   495,    15,    16,
     471,     3,   750,   474,   140,   413,     8,     9,    92,   448,
     133,   504,   505,    15,    16,    92,   448,   471,   132,   142,
     474,   448,   136,   363,   364,   518,   302,   303,   499,   369,
     370,   371,   471,    31,   374,   474,   374,   133,   526,   471,
     448,   539,   474,     3,   471,   499,   142,   474,     8,   797,
     798,     7,   629,     9,   137,   134,   554,   140,   546,   453,
     499,   559,    33,   471,    48,   405,   474,   499,     3,   392,
     818,    27,   499,     8,     9,   133,   135,   392,   576,   137,
      15,    16,     3,   423,    40,    15,    16,     8,     9,   429,
     413,   499,   432,   136,    15,    16,   436,   804,   413,     8,
       9,   132,   131,    59,    62,   136,    15,    16,    10,    11,
      12,    13,    14,   132,    41,    42,   392,   136,   695,    10,
      11,    12,    13,    14,   132,   448,   466,   132,   136,   602,
      55,   136,   630,   448,   662,   633,   659,   413,   132,   132,
     132,   132,   136,   136,   642,   136,   132,   135,   471,   132,
     136,   474,   137,   136,   652,   495,   471,   495,   629,   474,
     133,   134,   132,   132,   504,   505,   136,   136,   506,   507,
     132,    45,   448,   140,   136,   629,   499,   681,   116,   117,
     118,   119,   680,   681,   499,   133,   134,   131,   659,    89,
     629,   120,   121,   122,   123,   471,    89,   629,   474,   133,
     134,    80,   629,   107,   108,   659,    51,    52,    53,    54,
      78,   131,   131,   131,   554,   131,   554,    60,    89,   133,
     659,   629,   137,   499,   695,   137,   131,    31,    31,   623,
     137,    31,    31,     7,   732,    37,   734,   131,   711,     3,
       3,   695,    78,    78,    78,    78,    90,   587,   128,    31,
     142,    31,    31,   131,     9,    31,   695,     3,   131,    31,
       3,     7,   602,   695,     3,   132,   136,   607,   695,   609,
     133,    17,    18,    19,    20,    21,    22,    23,    24,    25,
     132,   136,    78,   137,   132,   136,    37,   695,   137,    50,
     137,    17,   133,   133,   133,   137,   133,     3,     8,   137,
      95,    17,     4,   131,   129,   136,   629,   137,    54,   141,
     137,   140,   136,   132,   629,   131,   137,   139,   131,   137,
     137,   133,   137,   137,   131,    71,   137,    73,   136,   134,
     131,   829,     3,    48,   132,    81,   124,   134,   132,   131,
     680,   681,   680,   681,   137,   843,   137,   137,    94,   136,
     848,   134,   137,   629,    72,   136,   134,   104,   133,   136,
     105,   132,   131,   136,    18,   115,   134,   137,   134,    31,
     137,   711,   695,   132,   136,   132,   136,   142,    72,   137,
     695,   125,    90,   137,   136,   131,    17,   136,     3,   145,
      18,    31,   134,    31,   140,   131,   142,   137,   136,   136,
     146,     3,   148,   137,    31,   133,   133,   136,   143,    17,
       3,    95,    31,   132,   126,   132,     8,   131,   127,   695,
     131,   134,    31,   763,   764,   132,     9,    47,   528,     3,
      55,   602,   712,     7,     8,     9,   537,   471,   474,   320,
     633,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,   448,    27,    28,    29,   680,   685,   199,   721,
      34,    35,    36,   195,    38,    39,   597,   724,   593,    43,
     810,   811,   651,    47,   814,    49,   797,   374,   552,    53,
      54,    55,    56,    57,    58,    59,   546,   563,   241,    63,
      64,    65,   416,    67,   730,   746,    70,    71,   766,    73,
      74,    75,    76,    77,   758,   846,    -1,    81,    -1,    83,
      -1,    85,    86,    87,    88,    -1,    -1,    91,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,   100,   101,   102,   103,
      -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    -1,   133,
      -1,    -1,    -1,   137,   138,     3,    -1,    -1,    -1,     7,
       8,     9,   146,    -1,   148,    -1,    -1,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,
      38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    71,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    -1,    85,    86,    87,
      88,    -1,    -1,    91,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,   100,   101,   102,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,   132,   133,    -1,    -1,    -1,   137,
     138,     3,    -1,    -1,    -1,     7,     8,     9,   146,    -1,
     148,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    -1,    85,    86,    87,    88,    -1,    -1,    91,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
     132,   133,    -1,    -1,    -1,     3,   138,    -1,    -1,     7,
       8,     9,    -1,    -1,   146,    -1,   148,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    27,
      28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,
      38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    71,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    -1,    85,    86,    87,
      88,    -1,    -1,    91,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,   100,   101,   102,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,    -1,    -1,    -1,   137,
     138,     3,    -1,    -1,    -1,     7,     8,     9,   146,    -1,
     148,    -1,    -1,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    54,    55,    56,    57,    58,    59,    -1,    -1,
      -1,    63,    64,    65,    -1,    67,    -1,    -1,    70,    71,
      -1,    73,    74,    75,    76,    77,    -1,    -1,    -1,    81,
      -1,    83,    -1,    85,    86,    87,    88,    -1,    -1,    91,
      -1,    93,    94,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,   105,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,   131,
      -1,   133,    -1,    -1,    -1,     3,   138,    -1,    -1,     7,
       8,     9,    -1,    -1,   146,    -1,   148,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    -1,    27,
      -1,    29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,
      38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,    53,    54,    55,    56,    57,
      58,    59,    -1,    -1,    -1,    63,    64,    65,    -1,    67,
      -1,    -1,    70,    71,    -1,    73,    74,    75,    76,    77,
      -1,    -1,    -1,    81,    -1,    83,    -1,    85,    86,    87,
      88,    -1,    -1,    91,    -1,    93,    94,    -1,    -1,    -1,
      -1,    99,   100,   101,   102,   103,    -1,   105,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,   131,    -1,   133,    -1,    -1,    -1,     3,
     138,    -1,    -1,     7,     8,     9,    -1,    -1,   146,    -1,
     148,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    -1,    27,    -1,    29,    -1,    -1,    -1,    -1,
      34,    35,    36,    -1,    38,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    -1,    49,    -1,    -1,    -1,    53,
      54,    -1,    56,    57,    58,    59,    -1,    -1,    -1,    -1,
      64,    65,    -1,    67,    -1,    -1,    -1,    71,    -1,    73,
      74,    75,    76,    77,    -1,    -1,    -1,    81,    -1,    83,
      -1,    85,    86,    87,    -1,    -1,    -1,    91,    -1,    93,
      94,    -1,    -1,    -1,    -1,    99,   100,   101,   102,   103,
      -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,     7,     8,
       9,    -1,    -1,    -1,    -1,    -1,    15,    16,    17,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   131,    27,    -1,
      29,    -1,    -1,    -1,   138,    34,    35,    36,    -1,    38,
      39,    -1,   146,    -1,   148,    -1,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    53,    -1,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    83,    -1,    85,    86,    87,    -1,
      -1,     3,    91,    -1,    93,     7,    -1,    -1,    -1,    -1,
      99,   100,   101,   102,   103,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    -1,   148,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,
      -1,     3,    -1,    -1,    -1,     7,    -1,    -1,    -1,    -1,
       3,    -1,    94,    -1,     7,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   131,
      -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,   140,    -1,
      -1,    54,    -1,    -1,   146,    -1,   148,    69,    -1,    71,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    71,    81,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,    -1,
      -1,    -1,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    -1,    -1,     3,    -1,    -1,    -1,     7,     8,
       9,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,   131,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,   131,    38,
      39,    -1,    -1,    -1,   146,    -1,   148,    -1,    47,    -1,
      49,    -1,    -1,   146,    53,   148,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    83,    -1,    85,    86,    87,    -1,
      -1,    -1,    91,    -1,    93,    -1,    -1,    -1,    -1,    -1,
      99,   100,   101,   102,   103,    -1,    -1,    -1,    -1,    -1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,   130,    -1,    -1,    -1,    27,    -1,    29,    -1,   138,
      -1,   140,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    57,    58,    59,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      27,    -1,    29,    -1,    -1,    -1,   138,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,
      87,    -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,   112,   113,    -1,     7,     8,
       9,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,    -1,
      -1,    -1,    -1,   130,   131,    -1,    -1,    -1,    27,    -1,
      29,   138,    31,    -1,    -1,    34,    35,    36,    -1,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    47,    -1,
      49,    -1,    -1,    -1,    53,    -1,    -1,    56,    57,    58,
      59,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,    68,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    -1,
      -1,    -1,    -1,    -1,    83,    -1,    85,    86,    87,    -1,
      -1,    -1,    91,    -1,    93,    -1,    -1,    -1,    -1,    -1,
      99,   100,   101,   102,   103,    -1,    -1,    -1,    -1,     3,
      -1,    -1,    -1,     7,     8,     9,    -1,    -1,    -1,    -1,
      -1,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   130,   131,    27,    -1,    29,    -1,    -1,    -1,   138,
      34,    35,    36,    -1,    38,    39,    -1,    -1,    42,    -1,
      -1,    -1,    -1,    47,    -1,    49,    -1,    -1,    -1,    53,
      -1,    -1,    56,    57,    58,    59,    -1,    -1,    -1,    -1,
      64,    65,    -1,    67,    -1,    -1,    -1,    -1,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    85,    86,    87,    -1,    -1,    -1,    91,    -1,    93,
      -1,    -1,    -1,    -1,    -1,    99,   100,   101,   102,   103,
      -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,     7,
       8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,   132,    27,
      -1,    29,    -1,   137,   138,    -1,    34,    35,    36,    -1,
      38,    39,    -1,    -1,    42,    -1,    -1,    -1,    -1,    47,
      -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,    57,
      58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,    87,
      -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,    -1,
      -1,    99,   100,   101,   102,   103,    -1,    -1,    -1,    -1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   130,    -1,   132,    27,    -1,    29,    -1,   137,
     138,    -1,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    51,
      -1,    53,    -1,    -1,    56,    57,    58,    59,    -1,    61,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      27,    -1,    29,    -1,    -1,    -1,   138,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,
      87,    -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    27,    -1,    29,    -1,    -1,
     137,   138,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    57,    58,    59,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      27,    -1,    29,    -1,    -1,   137,   138,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    42,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,
      87,    -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    27,    -1,    29,    -1,    -1,
      -1,   138,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    57,    58,    59,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,   104,    -1,    -1,    -1,     3,    -1,    -1,    -1,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      27,    -1,    29,    -1,    -1,    -1,   138,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,
      87,    -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    99,   100,   101,   102,   103,    -1,    -1,    -1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   130,    -1,    27,    -1,    29,    -1,    -1,
      -1,   138,    34,    35,    36,    -1,    38,    39,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    47,    -1,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    57,    58,    59,    -1,    -1,
      -1,    -1,    64,    65,    -1,    67,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    -1,    -1,    -1,    -1,
      -1,    83,    -1,    85,    86,    87,    -1,    -1,    -1,    91,
      -1,    93,    -1,    -1,    -1,    -1,    -1,    99,   100,   101,
     102,   103,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,
       7,     8,     9,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
      27,    -1,    29,    -1,    -1,    -1,   138,    34,    35,    36,
      -1,    38,    39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      47,    -1,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    64,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    -1,    -1,    -1,    -1,    -1,    83,    -1,    85,    86,
      87,    -1,    -1,    -1,    91,    -1,    93,    -1,    -1,    -1,
      -1,    -1,    99,   100,   101,   102,   103,    -1,    10,    11,
      12,    13,    14,    -1,    -1,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    34,   130,    36,    -1,    38,    39,    -1,    -1,
      -1,   138,    -1,    -1,    -1,    47,    48,    49,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    74,    -1,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    10,    11,    12,    13,
      14,    -1,    94,    17,    96,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      34,    -1,    36,    -1,    38,    39,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    47,    48,    49,    -1,    -1,    -1,    53,
      54,    -1,    -1,    -1,   136,    -1,   138,   139,    -1,    -1,
      64,    -1,    66,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      74,    -1,    76,    77,    -1,    -1,    -1,    81,    -1,    -1,
      -1,    85,    86,    87,    -1,    -1,    -1,    -1,    -1,    -1,
      94,    -1,    96,    -1,    -1,    -1,    -1,    -1,    10,    11,
      12,    13,    14,    -1,    -1,    17,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    34,    -1,    36,    -1,    38,    39,   132,    -1,
      -1,    -1,   136,    -1,   138,    47,    48,    49,    -1,    -1,
      -1,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    74,    -1,    76,    77,    -1,    -1,    -1,    81,
      -1,    -1,    -1,    85,    86,    87,    -1,    -1,    -1,    -1,
      -1,    -1,    94,    -1,    96,    -1,    -1,    -1,    -1,    -1,
      10,    11,    12,    13,    14,    -1,    -1,    17,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    34,    -1,    36,    -1,    38,    39,
     132,    -1,    -1,    -1,   136,    -1,   138,    47,    48,    49,
      -1,    -1,    -1,    53,    54,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    64,    -1,    66,    -1,    -1,    -1,
      -1,    71,    -1,    -1,    74,    -1,    76,    77,    -1,    -1,
      -1,    81,    -1,    -1,    -1,    85,    86,    87,    -1,    -1,
      -1,    -1,    -1,    -1,    94,    -1,    96,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,   138
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     7,   150,   131,   152,     0,     3,    17,   153,   154,
     155,    46,   133,   132,   153,    32,    51,    61,   156,    17,
      92,    92,    92,    31,   134,    33,   151,    52,   157,   158,
       3,     8,     9,    15,    16,   159,   169,   170,   386,   387,
      48,    62,   160,   135,   136,   131,   161,   162,   163,   169,
       3,     8,     9,    15,    16,   171,   172,   173,   175,   263,
     264,   265,   305,   306,   308,   309,   310,   311,   312,   322,
     356,   371,   382,   169,   132,   135,   163,    55,     3,     7,
       8,     9,    15,    16,    27,    29,    34,    35,    36,    38,
      39,    47,    49,    53,    56,    57,    58,    59,    64,    65,
      67,    74,    75,    76,    77,    83,    85,    86,    87,    91,
      93,    99,   100,   101,   102,   103,   112,   113,   130,   131,
     138,   176,   177,   178,   179,   180,   181,   184,   185,   186,
     187,   188,   193,   194,   198,   199,   200,   201,   202,   203,
     204,   205,   206,   207,   212,   213,   214,   215,   216,   219,
     220,   221,   222,   223,   224,   225,   267,   268,   269,   293,
     294,   296,   297,   298,   313,   317,   318,    31,    68,   176,
     313,    31,   313,     7,   267,   313,   267,   313,   172,   164,
     140,   137,    45,    89,    89,   131,    80,   131,    78,   131,
      60,    89,    78,    88,   131,   133,   226,   247,    78,   131,
     226,   247,   372,   383,     3,     8,     9,    15,    16,   176,
     267,   314,   315,   316,   386,    30,    84,    98,   218,   226,
     323,    51,    61,   176,   137,   137,   131,   319,   319,   176,
     319,   176,    31,   174,   176,   307,    40,   266,   267,   270,
     295,    31,   137,    31,   267,    31,   267,     7,   165,   176,
       8,     9,    15,    16,    37,   131,     3,   176,   189,   190,
     192,     3,   195,   196,   197,   389,   267,     3,   388,   389,
     176,   226,    42,   132,   137,   191,   192,   208,   209,   210,
       3,     7,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    28,    43,    54,    55,    63,    70,    71,    73,    81,
      94,   105,   131,   133,   137,   146,   148,   176,   177,   179,
     220,   222,   223,   224,   227,   230,   231,   232,   233,   234,
     238,   239,   240,   241,   243,   245,   246,   247,   248,   253,
     254,   258,   259,   293,   296,   298,   301,   302,   303,   304,
     324,   325,   326,   327,   328,   333,   334,   336,   337,   338,
     339,   342,   343,   344,   345,   346,   347,   348,   349,   352,
     353,   354,   390,    78,    78,   176,   132,   191,   208,    78,
      78,    90,   128,   132,   136,   142,     3,     7,    17,   217,
     326,    31,   176,   176,    10,    11,    12,    13,    14,   299,
     300,   299,   131,   176,   267,   320,   321,   324,   355,    31,
       8,    33,   357,    31,    31,    31,   131,   319,   266,     7,
     222,   301,    31,   131,   302,    31,   166,     3,     3,   182,
     183,   176,   132,   136,   133,   132,   136,   132,   136,    78,
     137,   132,   136,    44,    79,   132,   136,   142,   137,    17,
      18,   326,   340,   341,     4,   132,   335,   329,    50,   235,
      37,   226,   176,    41,    42,     3,   131,   132,   231,   296,
     350,   351,   231,   137,    17,   137,   141,   228,   136,    96,
     143,   236,    66,   144,   237,   235,   137,   140,   319,   176,
     176,   132,   176,   176,   176,     3,   324,   315,   386,   137,
     139,   324,   137,   230,   132,   136,   324,   137,   358,   131,
     355,   355,   176,    10,    11,    12,    13,    14,   271,   272,
     273,   275,   278,   279,   281,   282,   284,   137,   137,   301,
     302,     6,   167,   168,   327,   328,   133,   132,   136,   191,
     192,   326,   390,   137,   197,   389,   176,   137,   209,   211,
     191,   210,   324,     3,   136,   132,   136,   132,   136,     5,
      17,   330,   331,   238,   131,   226,   131,   249,   324,     3,
     132,   132,   132,   136,   134,   137,   299,     3,   176,   229,
     326,   390,   134,   137,   233,   234,   137,   114,   129,   384,
     300,   132,   321,     8,    95,   359,   360,    44,    79,   274,
     176,   299,   176,   299,   267,   267,   132,   136,   299,    17,
     326,   183,   136,   134,   134,   137,   228,   136,   324,   136,
      17,   341,     4,   133,   132,   330,   176,   255,   256,   257,
     267,   316,   321,     3,   137,   250,   251,   131,   351,   136,
     142,   137,    69,   140,   242,   244,   324,   116,   117,   118,
     119,   374,   131,   124,   376,    72,    48,   104,   361,   176,
      97,   276,    44,    79,   277,    44,    79,   280,   280,    44,
      79,   283,    44,    79,   285,   105,   286,   272,   134,   134,
     190,   137,   209,   132,   136,     5,    17,   327,   332,   132,
     136,   142,    26,    79,    82,   226,   252,   137,   132,   136,
     145,   260,   261,   231,   324,   137,   244,   115,   324,   385,
      18,   125,   377,    31,    72,   277,   324,   355,   301,   302,
      90,   136,   136,    17,   134,   256,   321,   252,   137,   250,
       3,   137,   262,   132,   136,   231,   120,   121,   122,   123,
     375,   132,   136,    18,    31,     8,    18,   106,   107,   108,
     109,   110,   111,   140,   362,   363,   364,   365,   366,    31,
     131,   287,   196,   136,   136,   262,   137,   261,   376,   324,
     324,    31,   133,   133,     3,     8,   367,   368,   369,   370,
     143,   363,   362,    17,    34,    36,    38,    39,    47,    48,
      49,    53,    54,    64,    66,    71,    74,    76,    77,    81,
      85,    86,    87,    94,    96,   132,   136,   138,   288,   289,
     290,   291,   292,   300,   136,    17,   250,     3,   377,    95,
       3,   104,   176,   176,    31,   147,   368,   362,   288,   132,
     289,   132,   132,   126,   378,     8,   176,   176,   134,    31,
     176,   139,   131,   127,   381,   134,   134,   134,   324,     3,
       8,   379,   380,   131,   373,   132,   136,   324,    31,   380,
     132,   324
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YYUSE (yytype);
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
/* Line 1787 of yacc.c  */
#line 427 "asn_grammar.y"
    {
	Module = new ModuleDefinition((yyvsp[(1) - (6)].sval), (yyvsp[(2) - (6)].slst), (Tag::Mode)(yyvsp[(4) - (6)].ival));
	ReferenceTokenContext = TYPEREFERENCE;
      }
    break;

  case 4:
/* Line 1787 of yacc.c  */
#line 436 "asn_grammar.y"
    {
	  (yyval.slst) = (yyvsp[(2) - (3)].slst);
	}
    break;

  case 5:
/* Line 1787 of yacc.c  */
#line 440 "asn_grammar.y"
    {
	  (yyval.slst) = new list<string>;
	}
    break;

  case 6:
/* Line 1787 of yacc.c  */
#line 447 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
	(yyval.slst)->push_front(*(yyvsp[(1) - (1)].sval));
      }
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 452 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].slst)->push_front(*(yyvsp[(1) - (2)].sval));
	(yyval.slst) = (yyvsp[(2) - (2)].slst);
      }
    break;

  case 9:
/* Line 1787 of yacc.c  */
#line 461 "asn_grammar.y"
    {
        ostringstream os;
	os << (int)(yyvsp[(1) - (1)].ival);
	(yyval.sval) = new string(os.str());
      }
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 471 "asn_grammar.y"
    {
	delete (yyvsp[(1) - (4)].sval);
	ostringstream os;
	os << (int)(yyvsp[(3) - (4)].ival);
	(yyval.sval) = new string(os.str());
      }
    break;

  case 12:
/* Line 1787 of yacc.c  */
#line 481 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Explicit;
      }
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 485 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Implicit;
      }
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 489 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Automatic;
      }
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 493 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Explicit;
      }
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 515 "asn_grammar.y"
    {
	Module->SetExports((yyvsp[(1) - (1)].tlst));
      }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 519 "asn_grammar.y"
    {
	Module->SetExportAll();
      }
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 544 "asn_grammar.y"
    {
	CurrentImportList = (yyvsp[(1) - (2)].tlst);
	ReferenceTokenContext = MODULEREFERENCE;
      }
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 549 "asn_grammar.y"
    {
	if (!HasObjectTypeMacro) {
	  HasObjectTypeMacro = (yyvsp[(1) - (4)].tlst)->GetValuesIndex(SearchType("OBJECT-TYPE")) != P_MAX_INDEX;
	  if (HasObjectTypeMacro)
	    PError << "Info: including OBJECT-TYPE macro" << endl;
	}
	Module->AddImport(new ImportModule((yyvsp[(4) - (4)].sval), (yyvsp[(1) - (4)].tlst)));
	ReferenceTokenContext = TYPEREFERENCE;
	CurrentImportList = NULL;
      }
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 564 "asn_grammar.y"
    {
	ReferenceTokenContext = TYPEREFERENCE;
        BraceTokenContext = OID_BRACE;
      }
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 569 "asn_grammar.y"
    {
        BraceTokenContext = '{';
	delete (yyvsp[(3) - (3)].vval);
      }
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 580 "asn_grammar.y"
    {
	(yyval.vval) = NULL;
      }
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 587 "asn_grammar.y"
    {
	(yyval.vval) = new DefinedValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 591 "asn_grammar.y"
    {
	(yyval.vval) = new DefinedValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 37:
/* Line 1787 of yacc.c  */
#line 600 "asn_grammar.y"
    {
	(yyval.tlst) = new TypesList;
	(yyval.tlst)->Append((yyvsp[(1) - (1)].tval));
      }
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 605 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].tlst)->Append((yyvsp[(1) - (3)].tval));
	(yyval.tlst) = (yyvsp[(3) - (3)].tlst);
      }
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 614 "asn_grammar.y"
    {
	(yyval.tval) = new ImportedType((yyvsp[(1) - (1)].sval), FALSE);
      }
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 645 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].tval)->SetName((yyvsp[(1) - (2)].sval));
	Module->AddType((yyvsp[(2) - (2)].tval));
	IdentifierTokenContext = (yyvsp[(2) - (2)].tval)->GetIdentifierTokenContext();
	BraceTokenContext = (yyvsp[(2) - (2)].tval)->GetBraceTokenContext();
      }
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 652 "asn_grammar.y"
    {
	IdentifierTokenContext = IDENTIFIER;
	BraceTokenContext = '{';
      }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 665 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].tval)->SetName((yyvsp[(1) - (3)].sval));
	Module->AddType((yyvsp[(3) - (3)].tval));
      }
    break;

  case 67:
/* Line 1787 of yacc.c  */
#line 689 "asn_grammar.y"
    { }
    break;

  case 83:
/* Line 1787 of yacc.c  */
#line 717 "asn_grammar.y"
    {
	(yyval.tval) = new DefinedType((yyvsp[(1) - (1)].sval), FALSE);
      }
    break;

  case 84:
/* Line 1787 of yacc.c  */
#line 721 "asn_grammar.y"
    {
	(yyval.tval) = new DefinedType((yyvsp[(1) - (1)].sval),
			     DummyParameters != NULL &&
			     find ( DummyParameters-> begin ( ), DummyParameters-> end ( ), *(yyvsp[(1) - (1)].sval)) != DummyParameters->end ( ));
      }
    break;

  case 86:
/* Line 1787 of yacc.c  */
#line 733 "asn_grammar.y"
    {
	*(yyvsp[(1) - (3)].sval) += *(yyvsp[(3) - (3)].sval);
	delete (yyvsp[(3) - (3)].sval);
      }
    break;

  case 87:
/* Line 1787 of yacc.c  */
#line 742 "asn_grammar.y"
    {
	(yyval.tval) = new BitStringType;
      }
    break;

  case 88:
/* Line 1787 of yacc.c  */
#line 746 "asn_grammar.y"
    {
	(yyval.tval) = new BitStringType((yyvsp[(4) - (5)].nlst));
      }
    break;

  case 89:
/* Line 1787 of yacc.c  */
#line 754 "asn_grammar.y"
    {
	(yyval.nlst) = new NamedNumberList;
	(yyval.nlst)->Append((yyvsp[(1) - (1)].nval));
      }
    break;

  case 90:
/* Line 1787 of yacc.c  */
#line 759 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].nlst)->InsertAt(0, (yyvsp[(3) - (3)].nval));
      }
    break;

  case 91:
/* Line 1787 of yacc.c  */
#line 766 "asn_grammar.y"
    {
	(yyval.nval) = new NamedNumber((yyvsp[(1) - (4)].sval), (int)(yyvsp[(3) - (4)].ival));
      }
    break;

  case 92:
/* Line 1787 of yacc.c  */
#line 770 "asn_grammar.y"
    {
	(yyval.nval) = new NamedNumber((yyvsp[(1) - (4)].sval), ((DefinedValue*)(yyvsp[(3) - (4)].vval))->GetReference());
	delete (yyvsp[(3) - (4)].vval);
      }
    break;

  case 93:
/* Line 1787 of yacc.c  */
#line 779 "asn_grammar.y"
    {
	(yyval.tval) = new BooleanType;
      }
    break;

  case 96:
/* Line 1787 of yacc.c  */
#line 792 "asn_grammar.y"
    {
	(yyval.tval) = new BMPStringType;
      }
    break;

  case 97:
/* Line 1787 of yacc.c  */
#line 796 "asn_grammar.y"
    {
	(yyval.tval) = new GeneralStringType;
      }
    break;

  case 98:
/* Line 1787 of yacc.c  */
#line 800 "asn_grammar.y"
    {
	(yyval.tval) = new GraphicStringType;
      }
    break;

  case 99:
/* Line 1787 of yacc.c  */
#line 804 "asn_grammar.y"
    {
	(yyval.tval) = new IA5StringType;
      }
    break;

  case 100:
/* Line 1787 of yacc.c  */
#line 808 "asn_grammar.y"
    {
	(yyval.tval) = new ISO646StringType;
      }
    break;

  case 101:
/* Line 1787 of yacc.c  */
#line 812 "asn_grammar.y"
    {
	(yyval.tval) = new NumericStringType;
      }
    break;

  case 102:
/* Line 1787 of yacc.c  */
#line 816 "asn_grammar.y"
    {
	(yyval.tval) = new PrintableStringType;
      }
    break;

  case 103:
/* Line 1787 of yacc.c  */
#line 820 "asn_grammar.y"
    {
	(yyval.tval) = new TeletexStringType;
      }
    break;

  case 104:
/* Line 1787 of yacc.c  */
#line 824 "asn_grammar.y"
    {
	(yyval.tval) = new T61StringType;
      }
    break;

  case 105:
/* Line 1787 of yacc.c  */
#line 828 "asn_grammar.y"
    {
	(yyval.tval) = new UniversalStringType;
      }
    break;

  case 106:
/* Line 1787 of yacc.c  */
#line 832 "asn_grammar.y"
    {
	(yyval.tval) = new VideotexStringType;
      }
    break;

  case 107:
/* Line 1787 of yacc.c  */
#line 836 "asn_grammar.y"
    {
	(yyval.tval) = new VisibleStringType;
      }
    break;

  case 108:
/* Line 1787 of yacc.c  */
#line 844 "asn_grammar.y"
    {
	(yyval.tval) = new UnrestrictedCharacterStringType;
      }
    break;

  case 109:
/* Line 1787 of yacc.c  */
#line 852 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(3) - (4)].tval);
      }
    break;

  case 110:
/* Line 1787 of yacc.c  */
#line 859 "asn_grammar.y"
    {
	(yyval.tval) = new ChoiceType((yyvsp[(1) - (1)].tlst));
      }
    break;

  case 111:
/* Line 1787 of yacc.c  */
#line 863 "asn_grammar.y"
    {
	(yyval.tval) = new ChoiceType((yyvsp[(1) - (3)].tlst), TRUE);
      }
    break;

  case 112:
/* Line 1787 of yacc.c  */
#line 867 "asn_grammar.y"
    {
	(yyval.tval) = new ChoiceType((yyvsp[(1) - (5)].tlst), TRUE, (yyvsp[(5) - (5)].tlst));
      }
    break;

  case 113:
/* Line 1787 of yacc.c  */
#line 874 "asn_grammar.y"
    {
	(yyval.tlst) = new TypesList;
	(yyval.tlst)->Append((yyvsp[(1) - (1)].tval));
      }
    break;

  case 114:
/* Line 1787 of yacc.c  */
#line 879 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].tlst)->Append((yyvsp[(3) - (3)].tval));
      }
    break;

  case 116:
/* Line 1787 of yacc.c  */
#line 892 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].tval)->SetName((yyvsp[(1) - (2)].sval));
	(yyval.tval) = (yyvsp[(2) - (2)].tval);
      }
    break;

  case 117:
/* Line 1787 of yacc.c  */
#line 897 "asn_grammar.y"
    {
	PError << StdError(Warning) << "unnamed field." << endl;
	ostringstream os;
	os << "_unnamed" << UnnamedFieldCount++;
	(yyvsp[(1) - (1)].tval)->SetName(new string(os.str()));
      }
    break;

  case 118:
/* Line 1787 of yacc.c  */
#line 909 "asn_grammar.y"
    {
	(yyval.tval) = new EmbeddedPDVType;
      }
    break;

  case 119:
/* Line 1787 of yacc.c  */
#line 917 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(3) - (4)].tval);
      }
    break;

  case 120:
/* Line 1787 of yacc.c  */
#line 924 "asn_grammar.y"
    {
	(yyval.tval) = new EnumeratedType((yyvsp[(1) - (1)].nlst), FALSE, NULL);
      }
    break;

  case 121:
/* Line 1787 of yacc.c  */
#line 928 "asn_grammar.y"
    {
	(yyval.tval) = new EnumeratedType((yyvsp[(1) - (5)].nlst), TRUE, NULL);
      }
    break;

  case 122:
/* Line 1787 of yacc.c  */
#line 932 "asn_grammar.y"
    {
	(yyval.tval) = new EnumeratedType((yyvsp[(1) - (7)].nlst), TRUE, (yyvsp[(7) - (7)].nlst));
      }
    break;

  case 123:
/* Line 1787 of yacc.c  */
#line 939 "asn_grammar.y"
    {
	(yyval.nlst) = new NamedNumberList;
	(yyval.nlst)->Append((yyvsp[(1) - (1)].nval));
      }
    break;

  case 124:
/* Line 1787 of yacc.c  */
#line 944 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].nlst)->Append((yyvsp[(3) - (3)].nval));
        PINDEX sz = (yyvsp[(1) - (3)].nlst)->GetSize();
        if (sz > 1)
          (yyvsp[(3) - (3)].nval)->SetAutoNumber((*(yyvsp[(1) - (3)].nlst))[sz-2]);
	(yyval.nlst) = (yyvsp[(1) - (3)].nlst);
      }
    break;

  case 125:
/* Line 1787 of yacc.c  */
#line 955 "asn_grammar.y"
    {
	(yyval.nval) = new NamedNumber((yyvsp[(1) - (1)].sval));
      }
    break;

  case 127:
/* Line 1787 of yacc.c  */
#line 964 "asn_grammar.y"
    {
	(yyval.tval) = new ExternalType;
      }
    break;

  case 128:
/* Line 1787 of yacc.c  */
#line 972 "asn_grammar.y"
    {
	(yyval.tval) = new AnyType(NULL);
      }
    break;

  case 129:
/* Line 1787 of yacc.c  */
#line 976 "asn_grammar.y"
    {
	(yyval.tval) = new AnyType((yyvsp[(4) - (4)].sval));
      }
    break;

  case 131:
/* Line 1787 of yacc.c  */
#line 989 "asn_grammar.y"
    {
	(yyval.tval) = new IntegerType;
      }
    break;

  case 132:
/* Line 1787 of yacc.c  */
#line 993 "asn_grammar.y"
    {
	(yyval.tval) = new IntegerType((yyvsp[(3) - (4)].nlst));
      }
    break;

  case 133:
/* Line 1787 of yacc.c  */
#line 1001 "asn_grammar.y"
    {
	(yyval.tval) = new NullType;
      }
    break;

  case 134:
/* Line 1787 of yacc.c  */
#line 1009 "asn_grammar.y"
    {
	(yyval.tval) = new ObjectClassFieldType((yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].sval));
      }
    break;

  case 135:
/* Line 1787 of yacc.c  */
#line 1017 "asn_grammar.y"
    {
	(yyval.tval) = new ObjectIdentifierType;
      }
    break;

  case 136:
/* Line 1787 of yacc.c  */
#line 1024 "asn_grammar.y"
    {
	(yyval.tval) = new OctetStringType;
      }
    break;

  case 137:
/* Line 1787 of yacc.c  */
#line 1032 "asn_grammar.y"
    {
	(yyval.tval) = new RealType;
      }
    break;

  case 138:
/* Line 1787 of yacc.c  */
#line 1040 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(3) - (4)].tval);
      }
    break;

  case 139:
/* Line 1787 of yacc.c  */
#line 1044 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType(NULL, FALSE, NULL);
      }
    break;

  case 140:
/* Line 1787 of yacc.c  */
#line 1048 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType(NULL, TRUE, NULL);
      }
    break;

  case 141:
/* Line 1787 of yacc.c  */
#line 1055 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType((yyvsp[(1) - (1)].tlst), FALSE, NULL);
      }
    break;

  case 142:
/* Line 1787 of yacc.c  */
#line 1059 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType((yyvsp[(1) - (3)].tlst), TRUE, NULL);
      }
    break;

  case 143:
/* Line 1787 of yacc.c  */
#line 1063 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType((yyvsp[(1) - (5)].tlst), TRUE, (yyvsp[(5) - (5)].tlst));
      }
    break;

  case 144:
/* Line 1787 of yacc.c  */
#line 1067 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceType(NULL, TRUE, (yyvsp[(3) - (3)].tlst));
      }
    break;

  case 145:
/* Line 1787 of yacc.c  */
#line 1074 "asn_grammar.y"
    {
	(yyval.tlst) = new TypesList;
	(yyval.tlst)->Append((yyvsp[(1) - (1)].tval));
      }
    break;

  case 146:
/* Line 1787 of yacc.c  */
#line 1079 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].tlst)->Append((yyvsp[(3) - (3)].tval));
      }
    break;

  case 148:
/* Line 1787 of yacc.c  */
#line 1087 "asn_grammar.y"
    {
	(yyvsp[(1) - (2)].tval)->SetOptional();
      }
    break;

  case 149:
/* Line 1787 of yacc.c  */
#line 1091 "asn_grammar.y"
    {
	IdentifierTokenContext = (yyvsp[(1) - (2)].tval)->GetIdentifierTokenContext();
      }
    break;

  case 150:
/* Line 1787 of yacc.c  */
#line 1095 "asn_grammar.y"
    {
	IdentifierTokenContext = IDENTIFIER;
	(yyvsp[(1) - (4)].tval)->SetDefaultValue((yyvsp[(4) - (4)].vval));
      }
    break;

  case 151:
/* Line 1787 of yacc.c  */
#line 1100 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(3) - (3)].tval);
      }
    break;

  case 152:
/* Line 1787 of yacc.c  */
#line 1108 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceOfType((yyvsp[(3) - (3)].tval), NULL);
      }
    break;

  case 153:
/* Line 1787 of yacc.c  */
#line 1116 "asn_grammar.y"
    {
	(yyval.tval) = new SetType((SequenceType*)(yyvsp[(3) - (4)].tval));
      }
    break;

  case 154:
/* Line 1787 of yacc.c  */
#line 1120 "asn_grammar.y"
    {
	(yyval.tval) = new SetType;
      }
    break;

  case 155:
/* Line 1787 of yacc.c  */
#line 1128 "asn_grammar.y"
    {
	(yyval.tval) = new SetOfType((yyvsp[(3) - (3)].tval), NULL);
      }
    break;

  case 156:
/* Line 1787 of yacc.c  */
#line 1136 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].tval)->SetTag((yyvsp[(1) - (2)].tagv).tagClass, (yyvsp[(1) - (2)].tagv).tagNumber, Module->GetDefaultTagMode());
	(yyval.tval) = (yyvsp[(2) - (2)].tval);
      }
    break;

  case 157:
/* Line 1787 of yacc.c  */
#line 1141 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].tval)->SetTag((yyvsp[(1) - (3)].tagv).tagClass, (yyvsp[(1) - (3)].tagv).tagNumber, Tag::Implicit);
	(yyval.tval) = (yyvsp[(3) - (3)].tval);
      }
    break;

  case 158:
/* Line 1787 of yacc.c  */
#line 1146 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].tval)->SetTag((yyvsp[(1) - (3)].tagv).tagClass, (yyvsp[(1) - (3)].tagv).tagNumber, Tag::Explicit);
	(yyval.tval) = (yyvsp[(3) - (3)].tval);
      }
    break;

  case 159:
/* Line 1787 of yacc.c  */
#line 1154 "asn_grammar.y"
    {
	(yyval.tagv).tagClass = (Tag::Type)(yyvsp[(2) - (4)].ival);
	(yyval.tagv).tagNumber = (int)(yyvsp[(3) - (4)].ival);
      }
    break;

  case 161:
/* Line 1787 of yacc.c  */
#line 1163 "asn_grammar.y"
    {
	if ((yyvsp[(1) - (1)].vval)->IsDescendant(IntegerValue::Class()))
	  (yyval.ival) = *(IntegerValue*)(yyvsp[(1) - (1)].vval);
	else
	  PError << StdError(Fatal) << "incorrect value type." << endl;
      }
    break;

  case 162:
/* Line 1787 of yacc.c  */
#line 1173 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Universal;
      }
    break;

  case 163:
/* Line 1787 of yacc.c  */
#line 1177 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Application;
      }
    break;

  case 164:
/* Line 1787 of yacc.c  */
#line 1181 "asn_grammar.y"
    {
	(yyval.ival) = Tag::Private;
      }
    break;

  case 165:
/* Line 1787 of yacc.c  */
#line 1185 "asn_grammar.y"
    {
	(yyval.ival) = Tag::ContextSpecific;
      }
    break;

  case 166:
/* Line 1787 of yacc.c  */
#line 1193 "asn_grammar.y"
    {
	(yyval.tval) = new SelectionType((yyvsp[(1) - (3)].sval), (yyvsp[(3) - (3)].tval));
      }
    break;

  case 167:
/* Line 1787 of yacc.c  */
#line 1201 "asn_grammar.y"
    {
	(yyval.tval) = new GeneralizedTimeType;
      }
    break;

  case 168:
/* Line 1787 of yacc.c  */
#line 1205 "asn_grammar.y"
    {
	(yyval.tval) = new UTCTimeType;
      }
    break;

  case 169:
/* Line 1787 of yacc.c  */
#line 1209 "asn_grammar.y"
    {
	(yyval.tval) = new ObjectDescriptorType;
      }
    break;

  case 170:
/* Line 1787 of yacc.c  */
#line 1217 "asn_grammar.y"
    { }
    break;

  case 173:
/* Line 1787 of yacc.c  */
#line 1230 "asn_grammar.y"
    { }
    break;

  case 174:
/* Line 1787 of yacc.c  */
#line 1232 "asn_grammar.y"
    { }
    break;

  case 176:
/* Line 1787 of yacc.c  */
#line 1245 "asn_grammar.y"
    {
	(yyvsp[(1) - (2)].tval)->AddConstraint((yyvsp[(2) - (2)].cons));
      }
    break;

  case 178:
/* Line 1787 of yacc.c  */
#line 1253 "asn_grammar.y"
    {
	(yyval.tval) = new SetOfType((yyvsp[(4) - (4)].tval), (yyvsp[(2) - (4)].cons));
      }
    break;

  case 179:
/* Line 1787 of yacc.c  */
#line 1257 "asn_grammar.y"
    {
	(yyval.tval) = new SetOfType((yyvsp[(4) - (4)].tval), new Constraint((yyvsp[(2) - (4)].elmt)));
      }
    break;

  case 180:
/* Line 1787 of yacc.c  */
#line 1261 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceOfType((yyvsp[(4) - (4)].tval), (yyvsp[(2) - (4)].cons));
      }
    break;

  case 181:
/* Line 1787 of yacc.c  */
#line 1265 "asn_grammar.y"
    {
	(yyval.tval) = new SequenceOfType((yyvsp[(4) - (4)].tval), new Constraint((yyvsp[(2) - (4)].elmt)));
      }
    break;

  case 182:
/* Line 1787 of yacc.c  */
#line 1272 "asn_grammar.y"
    {
	(yyval.cons) = (yyvsp[(2) - (4)].cons);
      }
    break;

  case 185:
/* Line 1787 of yacc.c  */
#line 1285 "asn_grammar.y"
    {
	(yyval.vval) = (yyvsp[(2) - (2)].vval);
      }
    break;

  case 186:
/* Line 1787 of yacc.c  */
#line 1289 "asn_grammar.y"
    {
	(yyval.vval) = NULL;
      }
    break;

  case 187:
/* Line 1787 of yacc.c  */
#line 1297 "asn_grammar.y"
    {
	(yyval.vval) = new IntegerValue((yyvsp[(1) - (1)].ival));
      }
    break;

  case 189:
/* Line 1787 of yacc.c  */
#line 1302 "asn_grammar.y"
    {
	delete (yyvsp[(1) - (3)].tval);
        PError << StdError(Warning) << "Typed exception unsupported" << endl;
	(yyval.vval) = (yyvsp[(3) - (3)].vval);
      }
    break;

  case 190:
/* Line 1787 of yacc.c  */
#line 1312 "asn_grammar.y"
    {
	(yyval.cons) = new Constraint((yyvsp[(1) - (1)].elst), FALSE, NULL);
      }
    break;

  case 191:
/* Line 1787 of yacc.c  */
#line 1316 "asn_grammar.y"
    {
	(yyval.cons) = new Constraint((yyvsp[(1) - (5)].elst), TRUE, NULL);
      }
    break;

  case 192:
/* Line 1787 of yacc.c  */
#line 1320 "asn_grammar.y"
    {
	(yyval.cons) = new Constraint(NULL, TRUE, (yyvsp[(5) - (5)].elst));
      }
    break;

  case 193:
/* Line 1787 of yacc.c  */
#line 1324 "asn_grammar.y"
    {
	(yyval.cons) = new Constraint((yyvsp[(1) - (6)].elst), TRUE, (yyvsp[(6) - (6)].elst));
      }
    break;

  case 195:
/* Line 1787 of yacc.c  */
#line 1333 "asn_grammar.y"
    {
	(yyval.elst) = new ConstraintElementList;
	(yyval.elst)->Append(new ConstrainAllConstraintElement((yyvsp[(2) - (2)].elmt)));
      }
    break;

  case 196:
/* Line 1787 of yacc.c  */
#line 1342 "asn_grammar.y"
    {
	(yyval.elst) = new ConstraintElementList;
	(yyval.elst)->Append(new ElementListConstraintElement((yyvsp[(1) - (1)].elst)));
      }
    break;

  case 197:
/* Line 1787 of yacc.c  */
#line 1347 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].elst)->Append(new ElementListConstraintElement((yyvsp[(3) - (3)].elst)));
      }
    break;

  case 198:
/* Line 1787 of yacc.c  */
#line 1354 "asn_grammar.y"
    {
	(yyval.elst) = new ConstraintElementList;
	(yyval.elst)->Append((yyvsp[(1) - (1)].elmt));
      }
    break;

  case 199:
/* Line 1787 of yacc.c  */
#line 1359 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].elst)->Append((yyvsp[(3) - (3)].elmt));
      }
    break;

  case 201:
/* Line 1787 of yacc.c  */
#line 1367 "asn_grammar.y"
    {
	(yyvsp[(1) - (2)].elmt)->SetExclusions((yyvsp[(2) - (2)].elmt));
      }
    break;

  case 202:
/* Line 1787 of yacc.c  */
#line 1374 "asn_grammar.y"
    {
	(yyval.elmt) = (yyvsp[(2) - (2)].elmt);
      }
    break;

  case 209:
/* Line 1787 of yacc.c  */
#line 1393 "asn_grammar.y"
    {
	(yyval.elmt) = new ElementListConstraintElement((yyvsp[(2) - (3)].elst));
      }
    break;

  case 210:
/* Line 1787 of yacc.c  */
#line 1401 "asn_grammar.y"
    {
	(yyval.elmt) = new SingleValueConstraintElement((yyvsp[(1) - (1)].vval));
      }
    break;

  case 211:
/* Line 1787 of yacc.c  */
#line 1405 "asn_grammar.y"
    {
	(yyval.elmt) = new SubTypeConstraintElement((yyvsp[(1) - (1)].tval));
      }
    break;

  case 216:
/* Line 1787 of yacc.c  */
#line 1417 "asn_grammar.y"
    {
	(yyval.elmt) = new ValueRangeConstraintElement((yyvsp[(1) - (4)].vval), (yyvsp[(4) - (4)].vval));
      }
    break;

  case 220:
/* Line 1787 of yacc.c  */
#line 1430 "asn_grammar.y"
    {
	(yyval.vval) = (yyvsp[(2) - (2)].vval);
      }
    break;

  case 222:
/* Line 1787 of yacc.c  */
#line 1438 "asn_grammar.y"
    {
	(yyval.vval) = new MinValue;
      }
    break;

  case 224:
/* Line 1787 of yacc.c  */
#line 1446 "asn_grammar.y"
    {
	(yyval.vval) = new MaxValue;
      }
    break;

  case 225:
/* Line 1787 of yacc.c  */
#line 1453 "asn_grammar.y"
    {
	(yyval.elmt) = new FromConstraintElement((yyvsp[(2) - (2)].cons));
      }
    break;

  case 226:
/* Line 1787 of yacc.c  */
#line 1460 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(2) - (2)].tval);
      }
    break;

  case 231:
/* Line 1787 of yacc.c  */
#line 1475 "asn_grammar.y"
    {
	(yyval.elmt) = new SizeConstraintElement((yyvsp[(2) - (2)].cons));
      }
    break;

  case 232:
/* Line 1787 of yacc.c  */
#line 1483 "asn_grammar.y"
    {
	(yyval.elmt) = new WithComponentConstraintElement(NULL, (yyvsp[(3) - (3)].cons), WithComponentConstraintElement::Default);
      }
    break;

  case 233:
/* Line 1787 of yacc.c  */
#line 1487 "asn_grammar.y"
    {
	(yyval.elmt) = (yyvsp[(3) - (3)].elmt);
      }
    break;

  case 234:
/* Line 1787 of yacc.c  */
#line 1494 "asn_grammar.y"
    {
	(yyval.elmt) = new InnerTypeConstraintElement((yyvsp[(2) - (3)].elst), FALSE);
      }
    break;

  case 235:
/* Line 1787 of yacc.c  */
#line 1498 "asn_grammar.y"
    {
	(yyval.elmt) = new InnerTypeConstraintElement((yyvsp[(6) - (7)].elst), TRUE);
      }
    break;

  case 236:
/* Line 1787 of yacc.c  */
#line 1505 "asn_grammar.y"
    {
	(yyval.elst) = new ConstraintElementList;
	(yyval.elst)->Append((yyvsp[(1) - (1)].elmt));
      }
    break;

  case 237:
/* Line 1787 of yacc.c  */
#line 1510 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].elst)->Append((yyvsp[(1) - (3)].elmt));
	(yyval.elst) = (yyvsp[(3) - (3)].elst);
      }
    break;

  case 238:
/* Line 1787 of yacc.c  */
#line 1518 "asn_grammar.y"
    {
	(yyval.elmt) = new WithComponentConstraintElement((yyvsp[(1) - (2)].sval), NULL, (int)(yyvsp[(2) - (2)].ival));
      }
    break;

  case 239:
/* Line 1787 of yacc.c  */
#line 1522 "asn_grammar.y"
    {
	(yyval.elmt) = new WithComponentConstraintElement((yyvsp[(1) - (3)].sval), (yyvsp[(2) - (3)].cons), (int)(yyvsp[(3) - (3)].ival));
      }
    break;

  case 240:
/* Line 1787 of yacc.c  */
#line 1529 "asn_grammar.y"
    {
	(yyval.ival) = WithComponentConstraintElement::Present;
      }
    break;

  case 241:
/* Line 1787 of yacc.c  */
#line 1533 "asn_grammar.y"
    {
	(yyval.ival) = WithComponentConstraintElement::Absent;
      }
    break;

  case 242:
/* Line 1787 of yacc.c  */
#line 1537 "asn_grammar.y"
    {
	(yyval.ival) = WithComponentConstraintElement::Optional;
      }
    break;

  case 243:
/* Line 1787 of yacc.c  */
#line 1541 "asn_grammar.y"
    {
	(yyval.ival) = WithComponentConstraintElement::Default;
      }
    break;

  case 246:
/* Line 1787 of yacc.c  */
#line 1554 "asn_grammar.y"
    {
      (yyval.cons) = new Constraint((yyvsp[(4) - (5)].elmt));
    }
    break;

  case 247:
/* Line 1787 of yacc.c  */
#line 1561 "asn_grammar.y"
    {
	(yyval.elmt) = new UserDefinedConstraintElement(NULL);
      }
    break;

  case 248:
/* Line 1787 of yacc.c  */
#line 1565 "asn_grammar.y"
    {
	(yyval.elmt) = new UserDefinedConstraintElement((yyvsp[(1) - (1)].tlst));
      }
    break;

  case 249:
/* Line 1787 of yacc.c  */
#line 1572 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].tlst)->Append((yyvsp[(1) - (3)].tval));
	(yyval.tlst) = (yyvsp[(3) - (3)].tlst);
      }
    break;

  case 250:
/* Line 1787 of yacc.c  */
#line 1577 "asn_grammar.y"
    {
	(yyval.tlst) = new TypesList;
	(yyval.tlst)->Append((yyvsp[(1) - (1)].tval));
      }
    break;

  case 251:
/* Line 1787 of yacc.c  */
#line 1585 "asn_grammar.y"
    {
	(yyval.tval) = (yyvsp[(3) - (3)].tval);
      }
    break;

  case 255:
/* Line 1787 of yacc.c  */
#line 1599 "asn_grammar.y"
    { (yyval.cons) = NULL; }
    break;

  case 262:
/* Line 1787 of yacc.c  */
#line 1622 "asn_grammar.y"
    { }
    break;

  case 263:
/* Line 1787 of yacc.c  */
#line 1627 "asn_grammar.y"
    { }
    break;

  case 264:
/* Line 1787 of yacc.c  */
#line 1632 "asn_grammar.y"
    { }
    break;

  case 265:
/* Line 1787 of yacc.c  */
#line 1638 "asn_grammar.y"
    { }
    break;

  case 271:
/* Line 1787 of yacc.c  */
#line 1652 "asn_grammar.y"
    {
	(yyval.sval) = ConcatNames((yyvsp[(1) - (3)].sval), '.', (yyvsp[(3) - (3)].sval));
      }
    break;

  case 272:
/* Line 1787 of yacc.c  */
#line 1659 "asn_grammar.y"
    {
	(yyval.sval) = new string("TYPE-IDENTIFIER");
      }
    break;

  case 273:
/* Line 1787 of yacc.c  */
#line 1663 "asn_grammar.y"
    {
	(yyval.sval) = new string("ABSTRACT-SYNTAX");
      }
    break;

  case 284:
/* Line 1787 of yacc.c  */
#line 1690 "asn_grammar.y"
    { }
    break;

  case 288:
/* Line 1787 of yacc.c  */
#line 1701 "asn_grammar.y"
    { }
    break;

  case 294:
/* Line 1787 of yacc.c  */
#line 1717 "asn_grammar.y"
    { }
    break;

  case 295:
/* Line 1787 of yacc.c  */
#line 1722 "asn_grammar.y"
    { }
    break;

  case 299:
/* Line 1787 of yacc.c  */
#line 1733 "asn_grammar.y"
    { }
    break;

  case 300:
/* Line 1787 of yacc.c  */
#line 1738 "asn_grammar.y"
    { }
    break;

  case 304:
/* Line 1787 of yacc.c  */
#line 1749 "asn_grammar.y"
    { }
    break;

  case 318:
/* Line 1787 of yacc.c  */
#line 1785 "asn_grammar.y"
    { }
    break;

  case 329:
/* Line 1787 of yacc.c  */
#line 1800 "asn_grammar.y"
    { }
    break;

  case 343:
/* Line 1787 of yacc.c  */
#line 1819 "asn_grammar.y"
    { }
    break;

  case 344:
/* Line 1787 of yacc.c  */
#line 1824 "asn_grammar.y"
    { }
    break;

  case 345:
/* Line 1787 of yacc.c  */
#line 1830 "asn_grammar.y"
    { }
    break;

  case 346:
/* Line 1787 of yacc.c  */
#line 1836 "asn_grammar.y"
    { }
    break;

  case 347:
/* Line 1787 of yacc.c  */
#line 1838 "asn_grammar.y"
    { }
    break;

  case 348:
/* Line 1787 of yacc.c  */
#line 1843 "asn_grammar.y"
    { }
    break;

  case 350:
/* Line 1787 of yacc.c  */
#line 1854 "asn_grammar.y"
    {
	(yyval.sval) = ConcatNames((yyvsp[(1) - (3)].sval), '.', (yyvsp[(3) - (3)].sval));
      }
    break;

  case 357:
/* Line 1787 of yacc.c  */
#line 1872 "asn_grammar.y"
    { }
    break;

  case 358:
/* Line 1787 of yacc.c  */
#line 1878 "asn_grammar.y"
    { }
    break;

  case 359:
/* Line 1787 of yacc.c  */
#line 1880 "asn_grammar.y"
    { }
    break;

  case 360:
/* Line 1787 of yacc.c  */
#line 1930 "asn_grammar.y"
    { }
    break;

  case 362:
/* Line 1787 of yacc.c  */
#line 1941 "asn_grammar.y"
    { }
    break;

  case 363:
/* Line 1787 of yacc.c  */
#line 1943 "asn_grammar.y"
    { }
    break;

  case 364:
/* Line 1787 of yacc.c  */
#line 1948 "asn_grammar.y"
    { }
    break;

  case 371:
/* Line 1787 of yacc.c  */
#line 1972 "asn_grammar.y"
    {
	DummyParameters = (yyvsp[(2) - (2)].slst);
      }
    break;

  case 372:
/* Line 1787 of yacc.c  */
#line 1976 "asn_grammar.y"
    {
	DummyParameters = NULL;
	(yyvsp[(5) - (5)].tval)->SetName((yyvsp[(1) - (5)].sval));
	(yyvsp[(5) - (5)].tval)->SetParameters((yyvsp[(2) - (5)].slst));
	Module->AddType((yyvsp[(5) - (5)].tval));
      }
    break;

  case 373:
/* Line 1787 of yacc.c  */
#line 1986 "asn_grammar.y"
    { }
    break;

  case 374:
/* Line 1787 of yacc.c  */
#line 1991 "asn_grammar.y"
    { }
    break;

  case 375:
/* Line 1787 of yacc.c  */
#line 1996 "asn_grammar.y"
    { }
    break;

  case 376:
/* Line 1787 of yacc.c  */
#line 2001 "asn_grammar.y"
    { }
    break;

  case 377:
/* Line 1787 of yacc.c  */
#line 2006 "asn_grammar.y"
    { }
    break;

  case 378:
/* Line 1787 of yacc.c  */
#line 2011 "asn_grammar.y"
    {
	(yyval.slst) = (yyvsp[(2) - (3)].slst);
      }
    break;

  case 379:
/* Line 1787 of yacc.c  */
#line 2018 "asn_grammar.y"
    {
	(yyval.slst) = (yyvsp[(1) - (3)].slst);
	(yyval.slst)->push_back(*(yyvsp[(3) - (3)].sval));
      }
    break;

  case 380:
/* Line 1787 of yacc.c  */
#line 2023 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
	(yyval.slst)->push_back(*(yyvsp[(1) - (1)].sval));
      }
    break;

  case 381:
/* Line 1787 of yacc.c  */
#line 2031 "asn_grammar.y"
    {
	(yyval.sval) = (yyvsp[(3) - (3)].sval);
      }
    break;

  case 383:
/* Line 1787 of yacc.c  */
#line 2039 "asn_grammar.y"
    { }
    break;

  case 384:
/* Line 1787 of yacc.c  */
#line 2041 "asn_grammar.y"
    { }
    break;

  case 385:
/* Line 1787 of yacc.c  */
#line 2047 "asn_grammar.y"
    {
	(yyval.tval) = new ParameterizedType((yyvsp[(1) - (2)].sval), (yyvsp[(2) - (2)].tlst));
      }
    break;

  case 388:
/* Line 1787 of yacc.c  */
#line 2060 "asn_grammar.y"
    {
	(yyval.tlst) = (yyvsp[(2) - (3)].tlst);
      }
    break;

  case 389:
/* Line 1787 of yacc.c  */
#line 2067 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].tlst)->Append((yyvsp[(3) - (3)].tval));
	(yyval.tlst) = (yyvsp[(1) - (3)].tlst);
      }
    break;

  case 390:
/* Line 1787 of yacc.c  */
#line 2072 "asn_grammar.y"
    {
	(yyval.tlst) = new TypesList;
	(yyval.tlst)->Append((yyvsp[(1) - (1)].tval));
      }
    break;

  case 392:
/* Line 1787 of yacc.c  */
#line 2081 "asn_grammar.y"
    { }
    break;

  case 393:
/* Line 1787 of yacc.c  */
#line 2083 "asn_grammar.y"
    { }
    break;

  case 394:
/* Line 1787 of yacc.c  */
#line 2085 "asn_grammar.y"
    { }
    break;

  case 395:
/* Line 1787 of yacc.c  */
#line 2099 "asn_grammar.y"
    {
	IdentifierTokenContext = (yyvsp[(2) - (2)].tval)->GetIdentifierTokenContext();
	BraceTokenContext = (yyvsp[(2) - (2)].tval)->GetBraceTokenContext();
	NullTokenContext = NULL_VALUE;
      }
    break;

  case 396:
/* Line 1787 of yacc.c  */
#line 2105 "asn_grammar.y"
    {
	(yyvsp[(5) - (5)].vval)->SetValueName((yyvsp[(1) - (5)].sval));
	Module->AddValue((yyvsp[(5) - (5)].vval));
	IdentifierTokenContext = IDENTIFIER;
	BraceTokenContext = '{';
	NullTokenContext = NULL_TYPE;
      }
    break;

  case 403:
/* Line 1787 of yacc.c  */
#line 2131 "asn_grammar.y"
    {
	(yyval.vval) = new IntegerValue((yyvsp[(1) - (1)].ival));
      }
    break;

  case 409:
/* Line 1787 of yacc.c  */
#line 2153 "asn_grammar.y"
    {
	(yyval.vval) = new DefinedValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 410:
/* Line 1787 of yacc.c  */
#line 2157 "asn_grammar.y"
    {
	(yyval.vval) = new DefinedValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 412:
/* Line 1787 of yacc.c  */
#line 2166 "asn_grammar.y"
    {
	*(yyvsp[(1) - (3)].sval) += *(yyvsp[(3) - (3)].sval);
	delete (yyvsp[(3) - (3)].sval);
      }
    break;

  case 413:
/* Line 1787 of yacc.c  */
#line 2175 "asn_grammar.y"
    {
        IdentifierTokenContext = OID_IDENTIFIER;
      }
    break;

  case 414:
/* Line 1787 of yacc.c  */
#line 2179 "asn_grammar.y"
    {
	(yyval.vval) = new ObjectIdentifierValue((yyvsp[(3) - (4)].slst));
	IdentifierTokenContext = IDENTIFIER;
      }
    break;

  case 415:
/* Line 1787 of yacc.c  */
#line 2194 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
	(yyval.slst)->push_back(*(yyvsp[(1) - (1)].sval));
      }
    break;

  case 416:
/* Line 1787 of yacc.c  */
#line 2199 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].slst)->push_front(*(yyvsp[(1) - (2)].sval));
	(yyval.slst) = (yyvsp[(2) - (2)].slst);
      }
    break;

  case 418:
/* Line 1787 of yacc.c  */
#line 2208 "asn_grammar.y"
    {
        ostringstream os;
	os << (int)(yyvsp[(1) - (1)].ival);
	(yyval.sval) = new string(os.str());
      }
    break;

  case 419:
/* Line 1787 of yacc.c  */
#line 2214 "asn_grammar.y"
    {
	delete (yyvsp[(1) - (4)].sval);
	(yyval.sval) = (yyvsp[(3) - (4)].sval);
      }
    break;

  case 420:
/* Line 1787 of yacc.c  */
#line 2222 "asn_grammar.y"
    {
        ostringstream os;
	os << (int)(yyvsp[(1) - (1)].ival);
	(yyval.sval) = new string(os.str());
      }
    break;

  case 423:
/* Line 1787 of yacc.c  */
#line 2234 "asn_grammar.y"
    {
	(yyval.vval) = new OctetStringValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 424:
/* Line 1787 of yacc.c  */
#line 2238 "asn_grammar.y"
    {
	(yyval.vval) = new OctetStringValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 425:
/* Line 1787 of yacc.c  */
#line 2245 "asn_grammar.y"
    {
	(yyval.vval) = new BitStringValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 426:
/* Line 1787 of yacc.c  */
#line 2249 "asn_grammar.y"
    {
	(yyval.vval) = new BitStringValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 427:
/* Line 1787 of yacc.c  */
#line 2253 "asn_grammar.y"
    {
	(yyval.vval) = new BitStringValue((yyvsp[(2) - (3)].slst));
      }
    break;

  case 428:
/* Line 1787 of yacc.c  */
#line 2257 "asn_grammar.y"
    {
	(yyval.vval) = new BitStringValue;
      }
    break;

  case 429:
/* Line 1787 of yacc.c  */
#line 2265 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
      }
    break;

  case 430:
/* Line 1787 of yacc.c  */
#line 2269 "asn_grammar.y"
    {
	// Look up $3
	(yyvsp[(1) - (3)].slst)->push_back(string());
      }
    break;

  case 431:
/* Line 1787 of yacc.c  */
#line 2278 "asn_grammar.y"
    {
	(yyval.vval) = new BooleanValue(TRUE);
      }
    break;

  case 432:
/* Line 1787 of yacc.c  */
#line 2282 "asn_grammar.y"
    {
	(yyval.vval) = new BooleanValue(FALSE);
      }
    break;

  case 434:
/* Line 1787 of yacc.c  */
#line 2297 "asn_grammar.y"
    {
	(yyval.vval) = new CharacterStringValue((yyvsp[(1) - (1)].sval));
      }
    break;

  case 438:
/* Line 1787 of yacc.c  */
#line 2307 "asn_grammar.y"
    {
	(yyval.vval) = new CharacterStringValue((yyvsp[(2) - (3)].slst));
      }
    break;

  case 439:
/* Line 1787 of yacc.c  */
#line 2314 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
	(yyval.slst)->push_back(*(yyvsp[(1) - (1)].sval));
      }
    break;

  case 440:
/* Line 1787 of yacc.c  */
#line 2319 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].slst)->push_back(*(yyvsp[(3) - (3)].sval));
      }
    break;

  case 442:
/* Line 1787 of yacc.c  */
#line 2327 "asn_grammar.y"
    {
	PError << StdError(Warning) << "DefinedValue in string unsupported" << endl;
      }
    break;

  case 443:
/* Line 1787 of yacc.c  */
#line 2334 "asn_grammar.y"
    {
	if ((yyvsp[(2) - (9)].ival) != 0 || (yyvsp[(4) - (9)].ival) != 0 || (yyvsp[(6) - (9)].ival) > 255 || (yyvsp[(8) - (9)].ival) > 255)
	  PError << StdError(Warning) << "Illegal value in Character Quadruple" << endl;
	(yyval.vval) = new CharacterValue((BYTE)(yyvsp[(2) - (9)].ival), (BYTE)(yyvsp[(4) - (9)].ival), (BYTE)(yyvsp[(6) - (9)].ival), (BYTE)(yyvsp[(8) - (9)].ival));
      }
    break;

  case 444:
/* Line 1787 of yacc.c  */
#line 2343 "asn_grammar.y"
    {
	if ((yyvsp[(2) - (5)].ival) > 255 || (yyvsp[(4) - (5)].ival) > 255)
	  PError << StdError(Warning) << "Illegal value in Character Tuple" << endl;
	(yyval.vval) = new CharacterValue((BYTE)(yyvsp[(2) - (5)].ival), (BYTE)(yyvsp[(4) - (5)].ival));
      }
    break;

  case 445:
/* Line 1787 of yacc.c  */
#line 2353 "asn_grammar.y"
    {
	(yyvsp[(3) - (3)].vval)->SetValueName((yyvsp[(1) - (3)].sval));
	(yyval.vval) = (yyvsp[(3) - (3)].vval);
      }
    break;

  case 446:
/* Line 1787 of yacc.c  */
#line 2362 "asn_grammar.y"
    {
	(yyval.vval) = new NullValue;
      }
    break;

  case 449:
/* Line 1787 of yacc.c  */
#line 2375 "asn_grammar.y"
    {
	(yyval.vval) = new RealValue(0);
      }
    break;

  case 450:
/* Line 1787 of yacc.c  */
#line 2385 "asn_grammar.y"
    {
	(yyval.vval) = new RealValue(0);
      }
    break;

  case 451:
/* Line 1787 of yacc.c  */
#line 2389 "asn_grammar.y"
    {
	(yyval.vval) = new RealValue(0);
      }
    break;

  case 452:
/* Line 1787 of yacc.c  */
#line 2397 "asn_grammar.y"
    {
	(yyval.vval) = new SequenceValue((yyvsp[(2) - (3)].vlst));
      }
    break;

  case 453:
/* Line 1787 of yacc.c  */
#line 2401 "asn_grammar.y"
    {
	(yyval.vval) = new SequenceValue;
      }
    break;

  case 454:
/* Line 1787 of yacc.c  */
#line 2408 "asn_grammar.y"
    {
	(yyval.vlst) = new ValuesList;
	(yyval.vlst)->Append((yyvsp[(1) - (1)].vval));
      }
    break;

  case 455:
/* Line 1787 of yacc.c  */
#line 2413 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].vlst)->Append((yyvsp[(3) - (3)].vval));
      }
    break;

  case 456:
/* Line 1787 of yacc.c  */
#line 2420 "asn_grammar.y"
    {
	(yyvsp[(2) - (2)].vval)->SetValueName((yyvsp[(1) - (2)].sval));
	(yyval.vval) = (yyvsp[(2) - (2)].vval);
      }
    break;

  case 458:
/* Line 1787 of yacc.c  */
#line 2477 "asn_grammar.y"
    { }
    break;

  case 462:
/* Line 1787 of yacc.c  */
#line 2498 "asn_grammar.y"
    {
	PError << StdError(Warning) << "MACRO unsupported" << endl;
      }
    break;

  case 463:
/* Line 1787 of yacc.c  */
#line 2505 "asn_grammar.y"
    {
	InMacroContext = TRUE;
      }
    break;

  case 464:
/* Line 1787 of yacc.c  */
#line 2509 "asn_grammar.y"
    {
	InMacroContext = FALSE;
      }
    break;

  case 465:
/* Line 1787 of yacc.c  */
#line 2513 "asn_grammar.y"
    {}
    break;

  case 466:
/* Line 1787 of yacc.c  */
#line 2515 "asn_grammar.y"
    {}
    break;

  case 476:
/* Line 1787 of yacc.c  */
#line 2565 "asn_grammar.y"
    {}
    break;

  case 477:
/* Line 1787 of yacc.c  */
#line 2567 "asn_grammar.y"
    {}
    break;

  case 478:
/* Line 1787 of yacc.c  */
#line 2569 "asn_grammar.y"
    {}
    break;

  case 493:
/* Line 1787 of yacc.c  */
#line 2597 "asn_grammar.y"
    {}
    break;

  case 494:
/* Line 1787 of yacc.c  */
#line 2602 "asn_grammar.y"
    {}
    break;

  case 495:
/* Line 1787 of yacc.c  */
#line 2610 "asn_grammar.y"
    {
	InMIBContext = TRUE;
      }
    break;

  case 496:
/* Line 1787 of yacc.c  */
#line 2620 "asn_grammar.y"
    {
	IdentifierTokenContext = OID_IDENTIFIER;
      }
    break;

  case 497:
/* Line 1787 of yacc.c  */
#line 2624 "asn_grammar.y"
    {
	Module->AddMIB(new MibObject((yyvsp[(1) - (16)].sval), (yyvsp[(5) - (16)].tval), (MibObject::Access)(yyvsp[(7) - (16)].ival), (MibObject::Status)(yyvsp[(9) - (16)].ival), (yyvsp[(10) - (16)].sval), (yyvsp[(11) - (16)].sval), (yyvsp[(12) - (16)].slst), (yyvsp[(13) - (16)].vval), (yyvsp[(16) - (16)].vval)));
	InMIBContext = FALSE;
	IdentifierTokenContext = IDENTIFIER;
      }
    break;

  case 498:
/* Line 1787 of yacc.c  */
#line 2633 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::read_only;
      }
    break;

  case 499:
/* Line 1787 of yacc.c  */
#line 2637 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::read_write;
      }
    break;

  case 500:
/* Line 1787 of yacc.c  */
#line 2641 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::write_only;
      }
    break;

  case 501:
/* Line 1787 of yacc.c  */
#line 2645 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::not_accessible;
      }
    break;

  case 502:
/* Line 1787 of yacc.c  */
#line 2652 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::mandatory;
      }
    break;

  case 503:
/* Line 1787 of yacc.c  */
#line 2656 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::optional;
      }
    break;

  case 504:
/* Line 1787 of yacc.c  */
#line 2660 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::obsolete;
      }
    break;

  case 505:
/* Line 1787 of yacc.c  */
#line 2664 "asn_grammar.y"
    {
	(yyval.ival) = MibObject::deprecated;
      }
    break;

  case 506:
/* Line 1787 of yacc.c  */
#line 2671 "asn_grammar.y"
    {
	(yyval.sval) = (yyvsp[(2) - (2)].sval);
      }
    break;

  case 507:
/* Line 1787 of yacc.c  */
#line 2675 "asn_grammar.y"
    {
	(yyval.sval) = NULL;
      }
    break;

  case 508:
/* Line 1787 of yacc.c  */
#line 2682 "asn_grammar.y"
    {
	(yyval.sval) = (yyvsp[(2) - (2)].sval);
      }
    break;

  case 509:
/* Line 1787 of yacc.c  */
#line 2686 "asn_grammar.y"
    {
	(yyval.sval) = NULL;
      }
    break;

  case 510:
/* Line 1787 of yacc.c  */
#line 2693 "asn_grammar.y"
    {
	(yyval.slst) = (yyvsp[(3) - (4)].slst);
      }
    break;

  case 511:
/* Line 1787 of yacc.c  */
#line 2697 "asn_grammar.y"
    {
	(yyval.slst) = NULL;
      }
    break;

  case 512:
/* Line 1787 of yacc.c  */
#line 2704 "asn_grammar.y"
    {
	(yyval.slst) = new list<string>;
	(yyval.slst)->push_back(*(yyvsp[(1) - (1)].sval));
      }
    break;

  case 513:
/* Line 1787 of yacc.c  */
#line 2709 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].slst)->push_back(*(yyvsp[(3) - (3)].sval));
      }
    break;

  case 516:
/* Line 1787 of yacc.c  */
#line 2721 "asn_grammar.y"
    {
	(yyval.vval) = (yyvsp[(3) - (4)].vval);
      }
    break;

  case 517:
/* Line 1787 of yacc.c  */
#line 2725 "asn_grammar.y"
    {
	(yyval.vval) = NULL;
      }
    break;

  case 518:
/* Line 1787 of yacc.c  */
#line 2732 "asn_grammar.y"
    {
	InMIBContext = TRUE;
	IdentifierTokenContext = OID_IDENTIFIER;
      }
    break;

  case 519:
/* Line 1787 of yacc.c  */
#line 2741 "asn_grammar.y"
    {
	Module->AddMIB(new MibTrap((yyvsp[(1) - (10)].sval), (yyvsp[(5) - (10)].vval), (yyvsp[(6) - (10)].vlst), (yyvsp[(7) - (10)].sval), (yyvsp[(8) - (10)].sval), (yyvsp[(10) - (10)].vval)));
	IdentifierTokenContext = IDENTIFIER;
	InMIBContext = FALSE;
      }
    break;

  case 520:
/* Line 1787 of yacc.c  */
#line 2750 "asn_grammar.y"
    {
	(yyval.vlst) = (yyvsp[(3) - (4)].vlst);
      }
    break;

  case 521:
/* Line 1787 of yacc.c  */
#line 2754 "asn_grammar.y"
    {
	(yyval.vlst) = NULL;
      }
    break;

  case 522:
/* Line 1787 of yacc.c  */
#line 2761 "asn_grammar.y"
    {
	(yyval.vlst) = new ValuesList;
	(yyval.vlst)->Append((yyvsp[(1) - (1)].vval));
      }
    break;

  case 523:
/* Line 1787 of yacc.c  */
#line 2766 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].vlst)->Append((yyvsp[(3) - (3)].vval));
      }
    break;

  case 529:
/* Line 1787 of yacc.c  */
#line 2807 "asn_grammar.y"
    {
	(yyval.tval) = new ImportedType((yyvsp[(1) - (3)].sval), TRUE);
      }
    break;

  case 530:
/* Line 1787 of yacc.c  */
#line 2815 "asn_grammar.y"
    {
	(yyval.nlst) = new NamedNumberList;
	(yyval.nlst)->Append((yyvsp[(1) - (1)].nval));
      }
    break;

  case 531:
/* Line 1787 of yacc.c  */
#line 2820 "asn_grammar.y"
    {
	(yyvsp[(1) - (3)].nlst)->Append((yyvsp[(3) - (3)].nval));
      }
    break;

  case 532:
/* Line 1787 of yacc.c  */
#line 2827 "asn_grammar.y"
    {
	(yyval.nval) = new NamedNumber((yyvsp[(1) - (4)].sval), (int)(yyvsp[(3) - (4)].ival));
      }
    break;

  case 533:
/* Line 1787 of yacc.c  */
#line 2831 "asn_grammar.y"
    {
	(yyval.nval) = new NamedNumber((yyvsp[(1) - (4)].sval), ((DefinedValue*)(yyvsp[(3) - (4)].vval))->GetReference());
	delete (yyvsp[(3) - (4)].vval);
      }
    break;

  case 535:
/* Line 1787 of yacc.c  */
#line 2841 "asn_grammar.y"
    {
	(yyval.ival) = -(yyvsp[(2) - (2)].ival);
      }
    break;


/* Line 1787 of yacc.c  */
#line 5462 "asn_grammar"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


