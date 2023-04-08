#include "parse.h"
#include "../../common/common_defs.h"

int sqlParse(const char *st, Query *&res);

int parse(const char *st, Query *&res) {
    sqlParse(st, res);
    if (res->getScf() == ScfError) {
        return 0;
    }
    return 1;
}

int *initIntsValue(int value) {
    int *res = new int;
    *res = value;
    return res;
}

float *initFloatValue(float value) {
    float *res = new float;
    *res = value;
    return res;
}

char *initCharsValue(const char *value) {
    char *res = strNew(value);
    return res;
}
// parse_defs
//----------------------------------------------------

char *strNew(const char *str) {
    int len = int(strlen(str));
    char *res = new char[len + 1];
    strcpy(res, str);
    res[len] = '\0';
    return res;
}