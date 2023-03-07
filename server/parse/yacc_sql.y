%{

#include "yacc_sql.hpp"
#include "lex_sql.h"
#include "parse_defs.h"
#include "../../src/common_defs.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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
	context->query->destroy();
	context->query=nullptr;
  }
  context->query=new ErrorQuery(str);
  context->condition_length = 0;
  context->from_length = 0;
  context->select_length = 0;
  context->value_length=0;
  context->value_tuple_num = 0;
  printf("parse sql failed. error=%s\n", str);
}

//todo:why??
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
	struct Condition* condition1;
	struct Value* value1;
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
%type <condition1> condition;
%type <value1> value;
%type <number> number;
%%
commands:
    /* empty */
    | commands command ;

command:
	select  
	| insert
	| update
	| delete
	| create_table
	| drop_table
	| show_tables
	| desc_table
	| create_index	
	| drop_index
	| sync
	| begin
	| commit
	| rollback
	| load_data
	| help
	| exit ;

exit:			
    EXIT SEMICOLON {
		// CONTEXT->query=new ExitQuery();
    };

help:
    HELP SEMICOLON {
		// CONTEXT->query=new HelpQuery();
    };

sync:
    SYNC SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_SYNC;
    }
    ;

begin:
    TRX_BEGIN SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_BEGIN;
    }
    ;

commit:
    TRX_COMMIT SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_COMMIT;
    }
    ;

rollback:
    TRX_ROLLBACK SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_ROLLBACK;
    }
    ;

drop_table:		/*drop table 语句的语法解析树*/
    DROP TABLE ID SEMICOLON {
        // CONTEXT->query_info->SCF_Flag = SCF_DROP_TABLE;//"drop_table";
        // drop_table_init(&CONTEXT->ssql->sstr.drop_table, $3);
    };

show_tables:
    SHOW TABLES SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_SHOW_TABLES;
    }
    ;

desc_table:
    DESC ID SEMICOLON {
    //   CONTEXT->query_info->SCF_Flag = SCF_DESC_TABLE;
    //   desc_table_init(&CONTEXT->ssql->sstr.desc_table, $2);
    }
    ;

create_index:		/*create index 语句的语法解析树*/
    CREATE INDEX ID ON ID LBRACE ID RBRACE SEMICOLON 
		{
			// CONTEXT->query_info->SCF_Flag = SCF_CREATE_INDEX;//"create_index";
			// create_index_init(&CONTEXT->ssql->sstr.create_index, $3, $5, $7);
		}
    ;

drop_index:			/*drop index 语句的语法解析树*/
    DROP INDEX ID  SEMICOLON 
		{
			// CONTEXT->query_info->SCF_Flag=SCF_DROP_INDEX;//"drop_index";
			// drop_index_init(&CONTEXT->ssql->sstr.drop_index, $3);
		}
    ;
create_table:		/*create table 语句的语法解析树*/
    CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE SEMICOLON {
		// CONTEXT->query=new CreateTableQuery();
		// CONTEXT->query->initialize();
		(static_cast<CreateTableQuery*>(CONTEXT->query))->setRelName($3);
		// //临时变量清零
		// CONTEXT->value_tuple_num=0;	
		CONTEXT->value_length = 0;
		}
    ;
attr_def_list:
    /* empty */
    | COMMA attr_def attr_def_list {  }
    ;
    
attr_def:
    ID_get type LBRACE number RBRACE {
		if(CONTEXT->query==nullptr){
			CONTEXT->query=new CreateTableQuery();
			CONTEXT->query->initialize();
		}
		AttrInfo attribute(CONTEXT->id,(AttrType)$2,$4);
		static_cast<CreateTableQuery*>(CONTEXT->query)->addAttr(attribute);
		CONTEXT->value_length++;
		}
    |ID_get type{
		//attr with default length
		if(CONTEXT->query==nullptr){
			CONTEXT->query=new CreateTableQuery();
			CONTEXT->query->initialize();
		}
		AttrInfo attribute(CONTEXT->id,(AttrType)$2,1);
		static_cast<CreateTableQuery*>(CONTEXT->query)->addAttr(attribute);
		CONTEXT->value_length++;
		}
    ;
number:
	NUMBER{
		$$ = $1;
		}
	;
type:
	INT_T{
		$$=INTS; 
		}
	|DATE_T{
		$$=DATES;
		}
    |STRING_T{
		$$=CHARS;
		}
    |FLOAT_T{ 
		$$=FLOATS; 
		}
    ;
ID_get:
	ID 
	{
		char *temp=$1; 
		snprintf(CONTEXT->id, sizeof(CONTEXT->id), "%s", temp);
	}
	;

	
insert:				/*insert   语句的语法解析树*/
    INSERT INTO ID VALUES LBRACE value value_list RBRACE value_unit SEMICOLON {
		// CONTEXT->query=new InsertQuery();
		// CONTEXT->query->initialize();


		// CONTEXT->query_info->SCF_Flag=SCF_INSERT;//"insert";
		// CONTEXT->value_tuple_num++;
		// inserts_init(&CONTEXT->ssql->sstr.insertion,$3,CONTEXT->values,CONTEXT->value_tuple_num,CONTEXT->value_length);
        // //临时变量清零
		// CONTEXT->value_tuple_num=0;
        // CONTEXT->value_length=0;
    }

value_unit:
	/* empty */
	|COMMA LBRACE value value_list RBRACE value_unit{
		// CONTEXT->value_tuple_num++;
	};
value_list:
    /* empty */
    | COMMA value value_list  {
  		// CONTEXT->values[CONTEXT->value_length++] = *$2;
	  }
    ;
value:
    NUMBER{	
  		// value_init_integer(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
	|DATE_STR{
		// $1=substr($1,1,strlen($1)-2);
		// value_init_date(&CONTEXT->values[CONTEXT->value_length++], $1);
	}
    |FLOAT{
  		// value_init_float(&CONTEXT->values[CONTEXT->value_length++],$1);
		}
    |SSS {
		// $1 = substr($1,1,strlen($1)-2);
  		// value_init_string(&CONTEXT->values[CONTEXT->value_length++], $1);
		}
    ;
    
delete:		/*  delete 语句的语法解析树*/
    DELETE FROM ID where SEMICOLON 
		{
			// CONTEXT->query_info->SCF_Flag = SCF_DELETE;//"delete";
			// deletes_init_relation(&CONTEXT->ssql->sstr.deletion, $3);
			// deletes_set_conditions(&CONTEXT->ssql->sstr.deletion, 
			// 		CONTEXT->conditions, CONTEXT->condition_length);
			// CONTEXT->condition_length = 0;	
    }
    ;
update:			/*  update 语句的语法解析树*/
    UPDATE ID SET ID EQ value where SEMICOLON
		{
			// CONTEXT->query_info->SCF_Flag = SCF_UPDATE;//"update";
			// Value *value = &CONTEXT->values[0];
			// updates_init(&CONTEXT->ssql->sstr.update, $2, $4, value, 
			// CONTEXT->conditions, CONTEXT->condition_length);
			// CONTEXT->condition_length = 0;
		}
    ;
select:				/*  select 语句的语法解析树*/
    SELECT select_attr FROM ID rel_list where SEMICOLON
		{
			// selects_append_relation(&CONTEXT->ssql->sstr.selection, $4);

			// selects_append_conditions(&CONTEXT->ssql->sstr.selection, CONTEXT->conditions, CONTEXT->condition_length);

			// CONTEXT->query_info->SCF_Flag=SCF_SELECT;//"select";

			// //临时变量清零
			// CONTEXT->condition_length=0;
			// CONTEXT->from_length=0;
			// CONTEXT->select_length=0;
			// CONTEXT->value_length = 0;
			// CONTEXT->value_tuple_num=0;
	}
	;

select_attr:
    STAR {  
			// RelAttr attr;
			// relation_attr_init(&attr, NULL, "*");
			// selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
    | ID attr_list {
			// RelAttr attr;
			// relation_attr_init(&attr, NULL, $1);
			// selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
  	| ID DOT ID attr_list {
			// RelAttr attr;
			// relation_attr_init(&attr, $1, $3);
			// selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
		}
    ;
attr_list:
    /* empty */
    | COMMA ID attr_list {
			// RelAttr attr;
			// relation_attr_init(&attr, NULL, $2);
			// selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
      }
    | COMMA ID DOT ID attr_list {
			// RelAttr attr;
			// relation_attr_init(&attr, $2, $4);
			// selects_append_attribute(&CONTEXT->ssql->sstr.selection, &attr);
  	  }
  	;

rel_list:
    /* empty */
    | COMMA ID rel_list {	
			// selects_append_relation(&CONTEXT->ssql->sstr.selection, $2);
		  }
    ;
where:
    /* empty */ 
    | WHERE condition condition_list {	
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
    ;
condition_list:
    /* empty */
    | AND condition condition_list {
				// CONTEXT->conditions[CONTEXT->condition_length++]=*$2;
			}
    ;
condition:
    ID comOp value 
		{
			// RelAttr left_attr;
			// relation_attr_init(&left_attr, NULL, $1);

			// Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
		|value comOp value 
		{
			// Value *left_value = &CONTEXT->values[CONTEXT->value_length - 2];
			// Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 0, NULL, right_value);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
		|ID comOp ID 
		{
			// RelAttr left_attr;
			// relation_attr_init(&left_attr, NULL, $1);
			// RelAttr right_attr;
			// relation_attr_init(&right_attr, NULL, $3);

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
    |value comOp ID
		{
			// Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];
			// RelAttr right_attr;
			// relation_attr_init(&right_attr, NULL, $3);

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
		}
    |ID DOT ID comOp value
		{
			// RelAttr left_attr;
			// relation_attr_init(&left_attr, $1, $3);
			// Value *right_value = &CONTEXT->values[CONTEXT->value_length - 1];

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 0, NULL, right_value);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
    |value comOp ID DOT ID
		{
			// Value *left_value = &CONTEXT->values[CONTEXT->value_length - 1];

			// RelAttr right_attr;
			// relation_attr_init(&right_attr, $3, $5);

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 0, NULL, left_value, 1, &right_attr, NULL);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
    |ID DOT ID comOp ID DOT ID
		{
			// RelAttr left_attr;
			// relation_attr_init(&left_attr, $1, $3);
			// RelAttr right_attr;
			// relation_attr_init(&right_attr, $5, $7);

			// Condition condition;
			// condition_init(&condition, CONTEXT->comp, 1, &left_attr, NULL, 1, &right_attr, NULL);
			// CONTEXT->conditions[CONTEXT->condition_length++] = condition;
    }
    ;

comOp:
  	  EQ { 
		// CONTEXT->comp = EQUAL_TO; 
		}
    | LT { 
		// CONTEXT->comp = LESS_THAN; 
		}
    | GT {
		//  CONTEXT->comp = GREAT_THAN; 
	}
    | LE {
		//  CONTEXT->comp = LESS_EQUAL;
		  }
    | GE {
		//  CONTEXT->comp = GREAT_EQUAL; 
		 }
    | NE {
		//  CONTEXT->comp = NOT_EQUAL; 
		 }
    ;

load_data:
		LOAD DATA INFILE SSS INTO TABLE ID SEMICOLON
		{
		//   CONTEXT->query_info->SCF_Flag = SCF_LOAD_DATA;
		// 	load_data_init(&CONTEXT->ssql->sstr.load_data, $7, $4);
		}
		;
%%

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);
// int sql_parse(const char *s, Query *sqls)
int sql_parse(const char *s,Query* & res){
	printf("sql parse begin\n");
	ParserContext context;
	memset(&context, 0, sizeof(context));
	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	scan_string(s, scanner);
	int result = yyparse(scanner);
	res=context.query;
	yylex_destroy(scanner);
	printf("sql parse end\n");
	return result;
}
