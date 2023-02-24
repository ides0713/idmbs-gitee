#include "parse.h"

int sql_parse(const char * st);

int parse(const char *st)
{
  sql_parse(st);
  // if (sqln->flag == SCF_ERROR)
  //   return SQL_SYNTAX;
  // else
  //   return SUCCESS;
  return 1;
}