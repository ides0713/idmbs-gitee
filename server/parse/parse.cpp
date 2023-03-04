#include "parse.h"

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
    printf("target str is %s\n",str);
    printf("strlen of target str is %d\n",strlen(str));
    int len=strlen(str);
    char* res=new char [len+1];
    strcpy(res,str);
    res[len]='\0';
    return res;
}

void nullnew(char * & n,const char *str){
    int len=strlen(str);
    n=new char [len+1];
    strcpy(n,str);
    n[len]='\0';
}