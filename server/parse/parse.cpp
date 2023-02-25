#include "parse.h"

int test_func(int param)
{
  printf("the param of the func is %d\n", param);
  return 1;
}

int sql_parse(const char *st);

int parse(const char *st)
{
  sql_parse(st);
  // if (sqln->flag == SCF_ERROR)
  //   return SQL_SYNTAX;
  // else
  //   return SUCCESS;
  return 1;
}
