#include<parse.h>
int parse(char *st);

int sql_parse(const char *s);

int parse(const char *st)
{
  sql_parse(st);
  printf("parse:parse\n");
  return 1;
}
