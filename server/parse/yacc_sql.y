%{

#include "yacc_sql.hpp"
#include "lex_sql.h"
#include "parse_defs.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct ParserContext {
	QueryInfo * query_info;
	size_t select_length,condition_length,from_length,value_length;
	Condition conditions[MAX_CONDITIONS_NUM];
	CompOp comp_op;
//   size_t select_length,condition_length,from_length,value_length,value_tuple_num;
//   Value values[MAX_NUM];
//   Condition conditions[MAX_NUM];
//   CompOp comp;
  	char rel_id[MAX_ID_LENGTH];
};

extern int yylex(void); 
extern int yyparse(void);

int yywrap()
{
	return 1;
}

void yyerror(yyscan_t scanner, const char *str)
{
  ParserContext *context = (ParserContext *)(yyget_extra(scanner));
//   query_reset(context->ssql);
  context->query_info->SCF_Flag = SCF_ERROR;
//   context->condition_length = 0;
//   context->from_length = 0;
//   context->select_length = 0;
//   context->value_length=0;
//   context->value_tuple_num = 0;
//   context->ssql->sstr.insertion.value_num = 0;
  printf("parse sql failed. error=%s", str);
}

ParserContext *get_context(yyscan_t scanner)
{
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)
%}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

%token SEMICOLON CREATE DROP TABLE TABLES INDEX SELECT DESC SHOW SYNC INSERT DELETE UPDATE LBRACE
       RBRACE COMMA TRX_BEGIN TRX_COMMIT TRX_ROLLBACK INT_T DATE_T STRING_T FLOAT_T HELP EXIT
       DOT INTO VALUES FROM WHERE AND SET ON LOAD DATA INFILE EQ LT GT LE GE NE

%union {
	struct RelAttr* attr;
	struct Condition* condition;
	struct Value* value;
	char* string;
	int number;
	float floats;
	char* position;
}

%token <number> NUMBER
%token <floats> FLOAT 
%token <string> ID
%token <string> PATH
%token <string> DATE_STR
%token <string> SSS
%token <string> STAR
%token <string> STRING_V

//non
%type <number> type;
%type <condition> _condition;
%type <value> _value;
%type <number> _number;
%%

commands:
	| commands command;

command: 
	select;

select:
	SELECT select_attr FROM ID rel_list where SEMICOLON
	{
		CONTEXT->query_info->SCF_Flag=SCF_SELECT;
	}

select_attr:
	STAR{}
	| ID attr_list{}
	| ID DOT ID attr_list {};

attr_list:
	| COMMA ID attr_list {}
	| COMMA ID DOT ID attr_list {};

rel_list:
	| COMMA ID rel_list {};

where:
	| WHERE condition condition_list {};

condition_list:
	| AND condition condition_list {};

condition:
	ID comOp value {}
	|value comOp value {}
	|ID comOp ID {}
	|value comOp ID{}
	|ID DOT ID comOp value{}
	|value comOp ID DOT ID{}
	|ID DOT ID comOp ID DOT ID{};

comOp:
  	EQ {}
    | LT {}
    | GT {}
    | LE {}
    | GE {}
    | NE {};

value:
    NUMBER{}
	|DATE_STR{}
    |FLOAT{}
    |SSS {};
%%

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);
// int sql_parse(const char *s, Query *sqls)
int sql_parse(const char *s,QueryInfo* res){
	ParserContext context;
	memset(&context, 0, sizeof(context));
	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	//res指向的为parse外部已经申请好空间的内存 数据通信
	context.query_info=res;
	scan_string(s, scanner);
	int result = yyparse(scanner);
	yylex_destroy(scanner);
	return result;
}
