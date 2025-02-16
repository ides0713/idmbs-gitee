/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "yacc_sql.y"


#include "yacc_sql.hpp"
#include "lex_sql.h"
#include "parse_defs.h"
#include "../../common/common_defs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <iostream>

struct ParserContext {
	Query * query=nullptr;
    size_t select_length,condition_length,from_length,value_length,value_tuple_num;
    Value values[MAX_ATTRS_NUM];
    Condition conditions[MAX_CONDITIONS_NUM];
    CompOp comp;
  	char id[MAX_ID_LENGTH];
};

extern int yylex(void); 
extern int yyparse(void);

int yywrap()
{
	return 1;
}

// char *substr(const char *s,int n1,int n2)/*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
// {
//   char * sp =new char [n2-n1+2];
// //   char *sp = (char *)malloc(sizeof(char) * (n2 - n1 + 2));
//   int i, j = 0;
//   for (i = n1; i <= n2; i++) {
//     sp[j++] = s[i];
//   }
//   sp[j] = 0;
//   return sp;
// }

void yyerror(yyscan_t scanner, const char *str)
{
  ParserContext *context = (ParserContext *)(yyget_extra(scanner));
  if(context->query!=nullptr){
	context->query->Destroy();
	context->query=nullptr;
  }
  context->query=new ErrorQuery(str);
  context->condition_length = 0;
  context->from_length = 0;
  context->select_length = 0;
  context->value_length=0;
  context->value_tuple_num = 0;
}

//todo:why??
ParserContext *get_context(yyscan_t scanner)
{
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)

#line 135 "yacc_sql.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc_sql.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_INDEX = 8,                      /* INDEX  */
  YYSYMBOL_SELECT = 9,                     /* SELECT  */
  YYSYMBOL_DESC = 10,                      /* DESC  */
  YYSYMBOL_SHOW = 11,                      /* SHOW  */
  YYSYMBOL_SYNC = 12,                      /* SYNC  */
  YYSYMBOL_INSERT = 13,                    /* INSERT  */
  YYSYMBOL_DELETE = 14,                    /* DELETE  */
  YYSYMBOL_UPDATE = 15,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 16,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 17,                    /* RBRACE  */
  YYSYMBOL_COMMA = 18,                     /* COMMA  */
  YYSYMBOL_TRX_BEGIN = 19,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 20,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 21,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 22,                     /* INT_T  */
  YYSYMBOL_DATE_T = 23,                    /* DATE_T  */
  YYSYMBOL_STRING_T = 24,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 25,                   /* FLOAT_T  */
  YYSYMBOL_HELP = 26,                      /* HELP  */
  YYSYMBOL_EXIT = 27,                      /* EXIT  */
  YYSYMBOL_DOT = 28,                       /* DOT  */
  YYSYMBOL_INTO = 29,                      /* INTO  */
  YYSYMBOL_VALUES = 30,                    /* VALUES  */
  YYSYMBOL_FROM = 31,                      /* FROM  */
  YYSYMBOL_WHERE = 32,                     /* WHERE  */
  YYSYMBOL_AND = 33,                       /* AND  */
  YYSYMBOL_SET = 34,                       /* SET  */
  YYSYMBOL_ON = 35,                        /* ON  */
  YYSYMBOL_LOAD = 36,                      /* LOAD  */
  YYSYMBOL_DATA = 37,                      /* DATA  */
  YYSYMBOL_INFILE = 38,                    /* INFILE  */
  YYSYMBOL_EQ = 39,                        /* EQ  */
  YYSYMBOL_LT = 40,                        /* LT  */
  YYSYMBOL_GT = 41,                        /* GT  */
  YYSYMBOL_LE = 42,                        /* LE  */
  YYSYMBOL_GE = 43,                        /* GE  */
  YYSYMBOL_NE = 44,                        /* NE  */
  YYSYMBOL_NUMBER = 45,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 46,                     /* FLOAT  */
  YYSYMBOL_ID = 47,                        /* ID  */
  YYSYMBOL_PATH = 48,                      /* PATH  */
  YYSYMBOL_DATE_STR = 49,                  /* DATE_STR  */
  YYSYMBOL_SSS = 50,                       /* SSS  */
  YYSYMBOL_STAR = 51,                      /* STAR  */
  YYSYMBOL_STRING_V = 52,                  /* STRING_V  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_commands = 54,                  /* commands  */
  YYSYMBOL_command = 55,                   /* command  */
  YYSYMBOL_exit = 56,                      /* exit  */
  YYSYMBOL_help = 57,                      /* help  */
  YYSYMBOL_sync = 58,                      /* sync  */
  YYSYMBOL_begin = 59,                     /* begin  */
  YYSYMBOL_commit = 60,                    /* commit  */
  YYSYMBOL_rollback = 61,                  /* rollback  */
  YYSYMBOL_drop_table = 62,                /* drop_table  */
  YYSYMBOL_show_tables = 63,               /* show_tables  */
  YYSYMBOL_desc_table = 64,                /* desc_table  */
  YYSYMBOL_create_index = 65,              /* create_index  */
  YYSYMBOL_drop_index = 66,                /* drop_index  */
  YYSYMBOL_create_table = 67,              /* create_table  */
  YYSYMBOL_attr_def_list = 68,             /* attr_def_list  */
  YYSYMBOL_attr_def = 69,                  /* attr_def  */
  YYSYMBOL_number = 70,                    /* number  */
  YYSYMBOL_type = 71,                      /* type  */
  YYSYMBOL_ID_get = 72,                    /* ID_get  */
  YYSYMBOL_insert = 73,                    /* insert  */
  YYSYMBOL_value_unit = 74,                /* value_unit  */
  YYSYMBOL_value_list = 75,                /* value_list  */
  YYSYMBOL_value = 76,                     /* value  */
  YYSYMBOL_delete = 77,                    /* delete  */
  YYSYMBOL_update = 78,                    /* update  */
  YYSYMBOL_select = 79,                    /* select  */
  YYSYMBOL_select_attr = 80,               /* select_attr  */
  YYSYMBOL_attr_list = 81,                 /* attr_list  */
  YYSYMBOL_rel_list = 82,                  /* rel_list  */
  YYSYMBOL_where = 83,                     /* where  */
  YYSYMBOL_condition_list = 84,            /* condition_list  */
  YYSYMBOL_condition = 85,                 /* condition  */
  YYSYMBOL_comOp = 86,                     /* comOp  */
  YYSYMBOL_load_data = 87                  /* load_data  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   160

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  80
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  171

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   307


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    98,    98,   100,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   122,   127,   132,   138,   144,   150,   156,   165,   171,
     178,   192,   198,   202,   204,   208,   216,   226,   231,   234,
     237,   240,   245,   253,   265,   267,   270,   272,   277,   281,
     286,   290,   298,   313,   322,   335,   343,   351,   360,   362,
     371,   382,   384,   388,   390,   394,   396,   401,   408,   415,
     422,   429,   436,   443,   453,   456,   459,   462,   465,   468,
     474
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "INDEX", "SELECT", "DESC", "SHOW", "SYNC",
  "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE", "COMMA", "TRX_BEGIN",
  "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "DATE_T", "STRING_T", "FLOAT_T",
  "HELP", "EXIT", "DOT", "INTO", "VALUES", "FROM", "WHERE", "AND", "SET",
  "ON", "LOAD", "DATA", "INFILE", "EQ", "LT", "GT", "LE", "GE", "NE",
  "NUMBER", "FLOAT", "ID", "PATH", "DATE_STR", "SSS", "STAR", "STRING_V",
  "$accept", "commands", "command", "exit", "help", "sync", "begin",
  "commit", "rollback", "drop_table", "show_tables", "desc_table",
  "create_index", "drop_index", "create_table", "attr_def_list",
  "attr_def", "number", "type", "ID_get", "insert", "value_unit",
  "value_list", "value", "delete", "update", "select", "select_attr",
  "attr_list", "rel_list", "where", "condition_list", "condition", "comOp",
  "load_data", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-141)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -141,     2,  -141,    12,    36,   -15,   -42,    18,    30,   -10,
      10,    11,    56,    61,    67,    74,    80,    51,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,    42,    49,    50,    52,
       6,  -141,    69,    95,    98,  -141,    55,    57,    71,  -141,
    -141,  -141,  -141,  -141,    65,    90,    72,   105,   106,    63,
      64,  -141,    66,  -141,  -141,    82,    83,    73,    68,    75,
      76,  -141,  -141,     9,    96,    99,   100,    16,   116,    85,
      92,  -141,   107,    70,   110,    81,  -141,  -141,    84,    83,
      41,  -141,  -141,     7,  -141,  -141,    13,    94,  -141,    41,
     123,    75,   113,  -141,  -141,  -141,  -141,   117,    87,    96,
      99,   129,   118,    88,  -141,  -141,  -141,  -141,  -141,  -141,
      22,    29,    16,  -141,    83,    91,   107,   134,    97,   122,
    -141,  -141,  -141,    41,   124,    13,  -141,  -141,   112,  -141,
      94,   140,   141,  -141,  -141,  -141,   128,   143,   118,   130,
      35,   102,  -141,  -141,  -141,  -141,  -141,  -141,   131,   147,
     125,  -141,  -141,    41,  -141,   104,   118,  -141,   135,   130,
    -141
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     3,    20,
      19,    14,    15,    16,    17,     9,    10,    11,    12,    13,
       8,     5,     7,     6,     4,    18,     0,     0,     0,     0,
      58,    55,     0,     0,     0,    23,     0,     0,     0,    24,
      25,    26,    22,    21,     0,     0,     0,     0,     0,     0,
       0,    56,     0,    29,    28,     0,    63,     0,     0,     0,
       0,    27,    31,    58,    58,    61,     0,     0,     0,     0,
       0,    42,    33,     0,     0,     0,    59,    57,     0,    63,
       0,    48,    50,     0,    49,    51,     0,    65,    52,     0,
       0,     0,     0,    38,    39,    40,    41,    36,     0,    58,
      61,     0,    46,     0,    74,    75,    76,    77,    78,    79,
       0,     0,     0,    64,    63,     0,    33,     0,     0,     0,
      60,    62,    54,     0,     0,     0,    69,    67,    70,    68,
      65,     0,     0,    34,    32,    37,     0,     0,    46,    44,
       0,     0,    66,    53,    80,    35,    30,    47,     0,     0,
       0,    71,    72,     0,    43,     0,    46,    73,     0,    44,
      45
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,    28,    54,  -141,  -141,  -141,
    -141,   -13,  -140,   -90,  -141,  -141,  -141,  -141,   -70,    47,
     -79,    19,    38,   -95,  -141
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,   102,    82,   146,   107,    83,
      31,   159,   134,    96,    32,    33,    34,    42,    61,    89,
      78,   123,    97,   120,    35
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
     112,   121,     2,    86,    87,    43,     3,     4,   157,   124,
     111,     5,     6,     7,     8,     9,    10,    11,    36,    46,
      37,    12,    13,    14,    59,    44,   168,    59,    15,    16,
     137,   139,    40,    45,    60,   113,    41,    85,    17,   130,
     150,    47,    38,   148,    39,   141,   114,   115,   116,   117,
     118,   119,   114,   115,   116,   117,   118,   119,    48,    49,
     161,    91,    92,    93,    50,    94,    95,    91,    92,   136,
      51,    94,    95,   166,    91,    92,   138,    52,    94,    95,
      91,    92,   160,    53,    94,    95,    91,    92,    54,    55,
      94,    95,   103,   104,   105,   106,    56,    57,    63,    58,
      62,    64,    65,    68,    66,    67,    69,    70,    71,    72,
      73,    74,    76,    75,    59,    77,    90,    88,    80,    98,
      79,   100,    81,    84,    99,   101,   108,   122,   109,   125,
     127,   110,   132,   128,   129,   135,   133,   144,   142,   147,
     151,   149,   145,   153,   154,   155,   156,   163,   158,   162,
     164,   167,   169,   165,   143,   126,   170,   131,     0,   152,
     140
};

static const yytype_int16 yycheck[] =
{
      90,    96,     0,    73,    74,    47,     4,     5,   148,    99,
      89,     9,    10,    11,    12,    13,    14,    15,     6,    29,
       8,    19,    20,    21,    18,     7,   166,    18,    26,    27,
     120,   121,    47,     3,    28,    28,    51,    28,    36,   109,
     135,    31,     6,   133,     8,   124,    39,    40,    41,    42,
      43,    44,    39,    40,    41,    42,    43,    44,    47,     3,
     150,    45,    46,    47,     3,    49,    50,    45,    46,    47,
       3,    49,    50,   163,    45,    46,    47,     3,    49,    50,
      45,    46,    47,     3,    49,    50,    45,    46,    37,    47,
      49,    50,    22,    23,    24,    25,    47,    47,     3,    47,
      31,     3,    47,    38,    47,    34,    16,    35,     3,     3,
      47,    47,    30,    47,    18,    32,    16,    18,    50,     3,
      47,    29,    47,    47,    39,    18,    16,    33,    47,     6,
      17,    47,     3,    16,    47,    47,    18,     3,    47,    17,
      28,    17,    45,     3,     3,    17,     3,    16,    18,    47,
       3,    47,    17,    28,   126,   101,   169,   110,    -1,   140,
     122
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    54,     0,     4,     5,     9,    10,    11,    12,    13,
      14,    15,    19,    20,    21,    26,    27,    36,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    73,    77,    78,    79,    87,     6,     8,     6,     8,
      47,    51,    80,    47,     7,     3,    29,    31,    47,     3,
       3,     3,     3,     3,    37,    47,    47,    47,    47,    18,
      28,    81,    31,     3,     3,    47,    47,    34,    38,    16,
      35,     3,     3,    47,    47,    47,    30,    32,    83,    47,
      50,    47,    69,    72,    47,    28,    81,    81,    18,    82,
      16,    45,    46,    47,    49,    50,    76,    85,     3,    39,
      29,    18,    68,    22,    23,    24,    25,    71,    16,    47,
      47,    83,    76,    28,    39,    40,    41,    42,    43,    44,
      86,    86,    33,    84,    76,     6,    69,    17,    16,    47,
      81,    82,     3,    18,    75,    47,    47,    76,    47,    76,
      85,    83,    47,    68,     3,    45,    70,    17,    76,    17,
      86,    28,    84,     3,     3,    17,     3,    75,    18,    74,
      47,    76,    47,    16,     3,    28,    76,    47,    75,    17,
      74
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    53,    54,    54,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    68,    69,    69,    70,    71,    71,
      71,    71,    72,    73,    74,    74,    75,    75,    76,    76,
      76,    76,    77,    78,    79,    80,    80,    80,    81,    81,
      81,    82,    82,    83,    83,    84,    84,    85,    85,    85,
      85,    85,    85,    85,    86,    86,    86,    86,    86,    86,
      87
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     4,     3,     3,
       9,     4,     8,     0,     3,     5,     2,     1,     1,     1,
       1,     1,     1,    10,     0,     6,     0,     3,     1,     1,
       1,     1,     5,     8,     7,     1,     2,     4,     0,     3,
       5,     0,     3,     0,     3,     0,     3,     3,     3,     3,
       3,     5,     5,     7,     1,     1,     1,     1,     1,     1,
       8
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        yyerror (scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (scanner);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, scanner);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *scanner)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *scanner)
{
  YY_USE (yyvaluep);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, scanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 21: /* exit: EXIT SEMICOLON  */
#line 122 "yacc_sql.y"
                   {
		// CONTEXT->query=new ExitQuery();
    }
#line 1335 "yacc_sql.cpp"
    break;

  case 22: /* help: HELP SEMICOLON  */
#line 127 "yacc_sql.y"
                   {
		// CONTEXT->query=new HelpQuery();
    }
#line 1343 "yacc_sql.cpp"
    break;

  case 23: /* sync: SYNC SEMICOLON  */
#line 132 "yacc_sql.y"
                   {
    //   CONTEXT->query_info->SCF_Flag = ScfSync;
    }
#line 1351 "yacc_sql.cpp"
    break;

  case 24: /* begin: TRX_BEGIN SEMICOLON  */
#line 138 "yacc_sql.y"
                        {
    //   CONTEXT->query_info->SCF_Flag = ScfBegin;
    }
#line 1359 "yacc_sql.cpp"
    break;

  case 25: /* commit: TRX_COMMIT SEMICOLON  */
#line 144 "yacc_sql.y"
                         {
    //   CONTEXT->query_info->SCF_Flag = ScfCommit;
    }
#line 1367 "yacc_sql.cpp"
    break;

  case 26: /* rollback: TRX_ROLLBACK SEMICOLON  */
#line 150 "yacc_sql.y"
                           {
    //   CONTEXT->query_info->SCF_Flag = ScfRollback;
    }
#line 1375 "yacc_sql.cpp"
    break;

  case 27: /* drop_table: DROP TABLE ID SEMICOLON  */
#line 156 "yacc_sql.y"
                            {
        if(CONTEXT->query==nullptr){
			CONTEXT->query=new DropTableQuery();
			CONTEXT->query->Init();
		}
        static_cast<DropTableQuery*>(CONTEXT->query)->SetRelName((yyvsp[-1].string));
    }
#line 1387 "yacc_sql.cpp"
    break;

  case 28: /* show_tables: SHOW TABLES SEMICOLON  */
#line 165 "yacc_sql.y"
                          {
    //   CONTEXT->query_info->SCF_Flag = ScfShowTables;
    }
#line 1395 "yacc_sql.cpp"
    break;

  case 29: /* desc_table: DESC ID SEMICOLON  */
#line 171 "yacc_sql.y"
                      {
    //   CONTEXT->query_info->SCF_Flag = ScfDescTable;
    //   desc_table_init(&CONTEXT->ssql->sstr.desc_table, $2);
    }
#line 1404 "yacc_sql.cpp"
    break;

  case 30: /* create_index: CREATE INDEX ID ON ID LBRACE ID RBRACE SEMICOLON  */
#line 178 "yacc_sql.y"
                                                     {
        // CONTEXT->query_info->SCF_Flag = ScfCreateIndex;//"create_index";
        // create_index_init(&CONTEXT->ssql->sstr.create_index, $3, $5, $7);
        if(CONTEXT->query==nullptr){
			CONTEXT->query=new CreateIndexQuery();
			CONTEXT->query->Init();
		}
        static_cast<CreateIndexQuery*>(CONTEXT->query)->SetIndexName((yyvsp[-6].string));
        RelAttr attr((yyvsp[-4].string),(yyvsp[-2].string));
        static_cast<CreateIndexQuery*>(CONTEXT->query)->SetAttr(attr);
	}
#line 1420 "yacc_sql.cpp"
    break;

  case 31: /* drop_index: DROP INDEX ID SEMICOLON  */
#line 192 "yacc_sql.y"
                             {
        // CONTEXT->query_info->SCF_Flag=ScfDropIndex;//"drop_index";
        // drop_index_init(&CONTEXT->ssql->sstr.drop_index, $3);
	}
#line 1429 "yacc_sql.cpp"
    break;

  case 32: /* create_table: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE SEMICOLON  */
#line 198 "yacc_sql.y"
                                                                   {
		(static_cast<CreateTableQuery*>(CONTEXT->query))->SetRelName((yyvsp[-5].string));
	}
#line 1437 "yacc_sql.cpp"
    break;

  case 34: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 204 "yacc_sql.y"
                                   {  }
#line 1443 "yacc_sql.cpp"
    break;

  case 35: /* attr_def: ID_get type LBRACE number RBRACE  */
#line 208 "yacc_sql.y"
                                     {
		if(CONTEXT->query==nullptr){
			CONTEXT->query=new CreateTableQuery();
			CONTEXT->query->Init();
		}
		AttrInfo attribute(CONTEXT->id,(AttrType)(yyvsp[-3].number),(yyvsp[-1].number));
		static_cast<CreateTableQuery*>(CONTEXT->query)->AddAttr(attribute);
	}
#line 1456 "yacc_sql.cpp"
    break;

  case 36: /* attr_def: ID_get type  */
#line 216 "yacc_sql.y"
                {
		if(CONTEXT->query==nullptr){
			CONTEXT->query=new CreateTableQuery();
			CONTEXT->query->Init();
		}
		AttrInfo attribute(CONTEXT->id,(AttrType)(yyvsp[0].number),4);
		static_cast<CreateTableQuery*>(CONTEXT->query)->AddAttr(attribute);
	}
#line 1469 "yacc_sql.cpp"
    break;

  case 37: /* number: NUMBER  */
#line 226 "yacc_sql.y"
              {
		(yyval.number) = (yyvsp[0].number);
	}
#line 1477 "yacc_sql.cpp"
    break;

  case 38: /* type: INT_T  */
#line 231 "yacc_sql.y"
             {
		(yyval.number)=AttrType::Ints;
	}
#line 1485 "yacc_sql.cpp"
    break;

  case 39: /* type: DATE_T  */
#line 234 "yacc_sql.y"
               {
		(yyval.number)=AttrType::Dates;
	}
#line 1493 "yacc_sql.cpp"
    break;

  case 40: /* type: STRING_T  */
#line 237 "yacc_sql.y"
             {
		(yyval.number)=AttrType::Chars;
	}
#line 1501 "yacc_sql.cpp"
    break;

  case 41: /* type: FLOAT_T  */
#line 240 "yacc_sql.y"
            {
		(yyval.number)=AttrType::Floats;
	}
#line 1509 "yacc_sql.cpp"
    break;

  case 42: /* ID_get: ID  */
#line 245 "yacc_sql.y"
           {
		char *temp=(yyvsp[0].string);
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
#line 1518 "yacc_sql.cpp"
    break;

  case 43: /* insert: INSERT INTO ID VALUES LBRACE value value_list RBRACE value_unit SEMICOLON  */
#line 253 "yacc_sql.y"
                                                                              {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new InsertQuery();
            CONTEXT->query->Init();
        }
        static_cast<InsertQuery*>(CONTEXT->query)->SetRelName((yyvsp[-7].string));
        static_cast<InsertQuery*>(CONTEXT->query)->AddValues(CONTEXT->value_length,CONTEXT->values);
        //临时变量清零
		CONTEXT->value_tuple_num=0;
        CONTEXT->value_length=0;
    }
#line 1534 "yacc_sql.cpp"
    break;

  case 45: /* value_unit: COMMA LBRACE value value_list RBRACE value_unit  */
#line 267 "yacc_sql.y"
                                                        {
		// CONTEXT->value_tuple_num++;
	}
#line 1542 "yacc_sql.cpp"
    break;

  case 47: /* value_list: COMMA value value_list  */
#line 272 "yacc_sql.y"
                              {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	}
#line 1550 "yacc_sql.cpp"
    break;

  case 48: /* value: NUMBER  */
#line 277 "yacc_sql.y"
          {
    	CONTEXT->values[CONTEXT->value_length].type=AttrType::Ints;
    	CONTEXT->values[CONTEXT->value_length++].data=InitIntsValue((yyvsp[0].number));
	}
#line 1559 "yacc_sql.cpp"
    break;

  case 49: /* value: DATE_STR  */
#line 281 "yacc_sql.y"
                 {
		//$1=substr($1,1,strlen($1)-2);
		//init_str
		// value_init_date(&CONTEXT->values[CONTEXT->value_length++], $1);
	}
#line 1569 "yacc_sql.cpp"
    break;

  case 50: /* value: FLOAT  */
#line 286 "yacc_sql.y"
          {
    	CONTEXT->values[CONTEXT->value_length].type=AttrType::Floats;
    	CONTEXT->values[CONTEXT->value_length++].data=InitFloatValue((yyvsp[0].floats));
	}
#line 1578 "yacc_sql.cpp"
    break;

  case 51: /* value: SSS  */
#line 290 "yacc_sql.y"
         {
    	(yyvsp[0].string) = SubStr((yyvsp[0].string),1,strlen((yyvsp[0].string))-2);
        CONTEXT->values[CONTEXT->value_length].type=AttrType::Chars;
        CONTEXT->values[CONTEXT->value_length++].data=InitCharsValue((yyvsp[0].string));
	}
#line 1588 "yacc_sql.cpp"
    break;

  case 52: /* delete: DELETE FROM ID where SEMICOLON  */
#line 298 "yacc_sql.y"
                                   {
        if(CONTEXT->query==nullptr){
			CONTEXT->query=new DeleteQuery();
			CONTEXT->query->Init();
		}
        static_cast<DeleteQuery*>(CONTEXT->query)->SetRelName((yyvsp[-2].string));
        static_cast<DeleteQuery*>(CONTEXT->query)->AddConditions(CONTEXT->condition_length,CONTEXT->conditions);
        // CONTEXT->query_info->SCF_Flag = ScfDelete;//"delete";
        // deletes_init_relation(&CONTEXT->ssql->sstr.deletion, $3);
        // deletes_set_conditions(&CONTEXT->ssql->sstr.deletion,
        // 		CONTEXT->conditions, CONTEXT->condition_length);
        CONTEXT->condition_length = 0;
    }
#line 1606 "yacc_sql.cpp"
    break;

  case 53: /* update: UPDATE ID SET ID EQ value where SEMICOLON  */
#line 313 "yacc_sql.y"
                                             {
        // CONTEXT->query_info->SCF_Flag = ScfUpdate;//"update";
        // Value *value = &CONTEXT->values[0];
        // updates_init(&CONTEXT->ssql->sstr.update, $2, $4, value,
        // CONTEXT->conditions, CONTEXT->condition_length);
        // CONTEXT->condition_length = 0;
	}
#line 1618 "yacc_sql.cpp"
    break;

  case 54: /* select: SELECT select_attr FROM ID rel_list where SEMICOLON  */
#line 322 "yacc_sql.y"
                                                       {
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelName((yyvsp[-3].string));
        static_cast<SelectQuery*>(CONTEXT->query)->AddConditions(CONTEXT->condition_length,CONTEXT->conditions);
        //临时变量清零
        CONTEXT->condition_length=0;
        // CONTEXT->from_length=0;
        // CONTEXT->select_length=0;
        // CONTEXT->value_length = 0;
        // CONTEXT->value_tuple_num=0;
	}
#line 1633 "yacc_sql.cpp"
    break;

  case 55: /* select_attr: STAR  */
#line 335 "yacc_sql.y"
         {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new SelectQuery();
            CONTEXT->query->Init();
        }
        RelAttr attr(nullptr,"*");
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelAttr(attr);
		}
#line 1646 "yacc_sql.cpp"
    break;

  case 56: /* select_attr: ID attr_list  */
#line 343 "yacc_sql.y"
                   {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new SelectQuery();
            CONTEXT->query->Init();
        }
        RelAttr attr(nullptr,(yyvsp[-1].string));
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelAttr(attr);
		}
#line 1659 "yacc_sql.cpp"
    break;

  case 57: /* select_attr: ID DOT ID attr_list  */
#line 351 "yacc_sql.y"
                              {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new SelectQuery();
            CONTEXT->query->Init();
        }
        RelAttr attr((yyvsp[-3].string),(yyvsp[-1].string));
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelAttr(attr);
		}
#line 1672 "yacc_sql.cpp"
    break;

  case 59: /* attr_list: COMMA ID attr_list  */
#line 362 "yacc_sql.y"
                         {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new SelectQuery();
            CONTEXT->query->Init();
        }
        RelAttr attr(nullptr,(yyvsp[-1].string));
        
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelAttr(attr);
    }
#line 1686 "yacc_sql.cpp"
    break;

  case 60: /* attr_list: COMMA ID DOT ID attr_list  */
#line 371 "yacc_sql.y"
                                {
        if(CONTEXT->query==nullptr){
            CONTEXT->query=new SelectQuery();
            CONTEXT->query->Init();
        }
        RelAttr attr((yyvsp[-3].string), (yyvsp[-1].string));
        
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelAttr(attr);
  	}
#line 1700 "yacc_sql.cpp"
    break;

  case 62: /* rel_list: COMMA ID rel_list  */
#line 384 "yacc_sql.y"
                        {
        static_cast<SelectQuery*>(CONTEXT->query)->AddRelName((yyvsp[-1].string));
	}
#line 1708 "yacc_sql.cpp"
    break;

  case 64: /* where: WHERE condition condition_list  */
#line 390 "yacc_sql.y"
                                     {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 1716 "yacc_sql.cpp"
    break;

  case 66: /* condition_list: AND condition condition_list  */
#line 396 "yacc_sql.y"
                                   {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
#line 1724 "yacc_sql.cpp"
    break;

  case 67: /* condition: ID comOp value  */
#line 401 "yacc_sql.y"
                   {
			RelAttr left_attr(NULL, (yyvsp[-2].string));
			Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
			Condition condition;
			condition.Init(CONTEXT->comp, 1, &left_attr,nullptr, 0, nullptr, right_value);
			CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 1736 "yacc_sql.cpp"
    break;

  case 68: /* condition: value comOp value  */
#line 408 "yacc_sql.y"
                           {
        Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
        Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
        Condition condition;
        condition.Init(CONTEXT->comp, 0,nullptr, left_value, 0,nullptr, right_value);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 1748 "yacc_sql.cpp"
    break;

  case 69: /* condition: ID comOp ID  */
#line 415 "yacc_sql.y"
                     {
        RelAttr left_attr(nullptr,(yyvsp[-2].string));
        RelAttr right_attr(nullptr,(yyvsp[0].string));
        Condition condition;
        condition.Init(CONTEXT->comp, 1, &left_attr,nullptr, 1, &right_attr,nullptr);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 1760 "yacc_sql.cpp"
    break;

  case 70: /* condition: value comOp ID  */
#line 422 "yacc_sql.y"
                   {
        Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
        RelAttr right_attr(nullptr, (yyvsp[0].string));
        Condition condition;
        condition.Init(CONTEXT->comp, 0,nullptr, left_value, 1, &right_attr,nullptr);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
#line 1772 "yacc_sql.cpp"
    break;

  case 71: /* condition: ID DOT ID comOp value  */
#line 429 "yacc_sql.y"
                          {
        RelAttr left_attr((yyvsp[-4].string), (yyvsp[-2].string));
        Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];
        Condition condition;
        condition.Init(CONTEXT->comp, 1, &left_attr,nullptr, 0, nullptr, right_value);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 1784 "yacc_sql.cpp"
    break;

  case 72: /* condition: value comOp ID DOT ID  */
#line 436 "yacc_sql.y"
                          {
        Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
        RelAttr right_attr((yyvsp[-2].string),(yyvsp[0].string));
        Condition condition;
        condition.Init(CONTEXT->comp, 0,nullptr, left_value, 1, &right_attr, nullptr);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 1796 "yacc_sql.cpp"
    break;

  case 73: /* condition: ID DOT ID comOp ID DOT ID  */
#line 443 "yacc_sql.y"
                              {
        RelAttr left_attr((yyvsp[-6].string), (yyvsp[-4].string));
        RelAttr right_attr((yyvsp[-2].string), (yyvsp[0].string));
        Condition condition;
        condition.Init(CONTEXT->comp, 1, &left_attr,nullptr, 1, &right_attr,nullptr);
        CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
#line 1808 "yacc_sql.cpp"
    break;

  case 74: /* comOp: EQ  */
#line 453 "yacc_sql.y"
           {
	    CONTEXT->comp = CompOp::EqualTo;
	}
#line 1816 "yacc_sql.cpp"
    break;

  case 75: /* comOp: LT  */
#line 456 "yacc_sql.y"
         {
		CONTEXT->comp = CompOp::LessThan;
	}
#line 1824 "yacc_sql.cpp"
    break;

  case 76: /* comOp: GT  */
#line 459 "yacc_sql.y"
         {
		CONTEXT->comp = CompOp::GreatThan;
	}
#line 1832 "yacc_sql.cpp"
    break;

  case 77: /* comOp: LE  */
#line 462 "yacc_sql.y"
         {
		CONTEXT->comp = CompOp::LessEqual;
	}
#line 1840 "yacc_sql.cpp"
    break;

  case 78: /* comOp: GE  */
#line 465 "yacc_sql.y"
         {
	    CONTEXT->comp = CompOp::GreatEqual;
	}
#line 1848 "yacc_sql.cpp"
    break;

  case 79: /* comOp: NE  */
#line 468 "yacc_sql.y"
         {
		CONTEXT->comp = CompOp::NotEqual;
	}
#line 1856 "yacc_sql.cpp"
    break;

  case 80: /* load_data: LOAD DATA INFILE SSS INTO TABLE ID SEMICOLON  */
#line 475 "yacc_sql.y"
                {
		//   CONTEXT->query_info->SCF_Flag = ScfLoadData;
		// 	load_data_init(&CONTEXT->ssql->sstr.load_data, $7, $4);
		}
#line 1865 "yacc_sql.cpp"
    break;


#line 1869 "yacc_sql.cpp"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (scanner, YY_("syntax error"));
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
                      yytoken, &yylval, scanner);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 480 "yacc_sql.y"


//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);
// int sql_parse(const char *s, Query *sqls)
int SqlParse(const char *s,Query* & res){
	ParserContext context;
	memset(&context, 0, sizeof(context));
	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	scan_string(s, scanner);
	int result = yyparse(scanner);
	res=context.query;
	yylex_destroy(scanner);
	return result;
}
