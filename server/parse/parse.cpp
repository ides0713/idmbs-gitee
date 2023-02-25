#include "parse.h"

int sql_parse(const char *st,QueryInfo* res);

int parse(const char *st,QueryInfo* res)
{
  sql_parse(st,res);
  // if (sqln->flag == SCF_ERROR)
  //   return SQL_SYNTAX;
  // else
  //   return SUCCESS;
  return 1;
}

//parse_defs
//----------------------------------------------------
int test_func(int param){
  printf("param of the func is %d\n",param);
  return 0;
}