#include "parse.h"

int sql_parse(const char *st,Query* &res);

int parse(const char *st,Query* &res)
{
  sql_parse(st,res);
  if (res->getSCFFlag() == SCF_ERROR)
    return 0;
  else
    return 1;
}

//parse_defs
//----------------------------------------------------
int test_func(int param){
  printf("param of the func is %d\n",param);
  return 0;
}