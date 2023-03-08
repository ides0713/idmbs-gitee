#include "parse.h"
#include <stdio.h>
int sql_parse(const char *st,Query* &res);

int parse(const char *st,Query* &res)
{
  sql_parse(st,res);
  if (res->getSCFFlag() == SCF_ERROR)
  {
    return 0;
  }
  return 1;
}

//parse_defs
//----------------------------------------------------
int test_func(int param){
  printf("param of the func is %d\n",param);
  return 0;
}

char * strnew(const char * str){
    int len=strlen(str);
    char* res=new char [len+1];
    strcpy(res,str);
    res[len]='\0';
    return res;
}