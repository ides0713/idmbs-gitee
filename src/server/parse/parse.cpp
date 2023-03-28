#include "parse.h"
#include <cstdio>
#include "../../common/common_defs.h"

int sqlParse(const char *st, Query *&res);

int parse(const char *st, Query *&res) {
    sqlParse(st, res);
    if (res->getScf() == ScfError) {
        return 0;
    }
    return 1;
}

// parse_defs
//----------------------------------------------------
[[maybe_unused]] int testFunc(int param) {
    debugPrint("param of the func is %d\n", param);
    return 0;
}

char *strnew(const char *str) {
    int len = int(strlen(str));
    char *res = new char[len + 1];
    strcpy(res, str);
    res[len] = '\0';
    return res;
}