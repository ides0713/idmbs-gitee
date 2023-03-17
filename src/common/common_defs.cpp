#include "common_defs.h"
#include <string.h>
#include <stdio.h>
char *substr(const char *s, int n1, int n2) /*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
{
    char *sp = new char[n2 - n1 + 2];
    //   char *sp = (char *)malloc(sizeof(char) * (n2 - n1 + 2));
    int i, j = 0;
    for (i = n1; i <= n2; i++)
    {
        sp[j++] = s[i];
    }
    sp[j] = 0;
    return sp;
}

message::message(int type, const char *message) : type_(type)
{
    strcpy(message_, message);
}