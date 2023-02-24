#include "parse.h"
int test_func(){
    printf("test success\n");
    return 0;
}
int sql_parse(const char * st);

int parse(const char *st)
{
  printf("parse:parse\n");
  sql_parse(st);
  printf("parse:parse\n");
  return 1;
}