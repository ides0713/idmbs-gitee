%{

#include "yacc_sql.hpp"
#include "lex_sql.h"

//在lex.yy.c里定义，会被yyparse()调用。在此声明消除编译和链接错误。
extern int yylex(void); 
// 在此声明，消除yacc生成代码时的告警
extern int yyparse(void); 
int yywrap()
{
	return 1;
}
// 该函数在y.tab.c里会被调用，需要在此定义
void yyerror(const char *s)
{
	printf("[error] %sn", s);
}
%}

%token NUMBER TOKHEAT STATE TOKTARGET TOKTEMPERATURE

%%
commands: /* empty */
| commands command
;
 
command: heat_switch | target_set ;

heat_switch:
TOKHEAT STATE
{
	printf("tHeat turned on or offn");
};

target_set:
TOKTARGET TOKTEMPERATURE NUMBER
{
	printf("tTemperature setn");
};
%%
//_____________________________________________________________________
// extern void scan_string(const char *str, yyscan_t scanner);

// Query* sqls
int sql_parse(const char *s){
	// ParserContext context;
	// memset(&context, 0, sizeof(context));
	YY_BUFFER_STATE bp = yy_scan_string(s);
    yy_switch_to_buffer(bp);
	int result = yyparse();
	yy_delete_buffer(bp);
	yylex_destroy();
	return result;
}