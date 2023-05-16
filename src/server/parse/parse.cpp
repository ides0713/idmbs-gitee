#include "parse.h"
#include "parse_defs.h"// for CompOp, AttrType, Chars, Dates, EqualTo, Floats
#include <assert.h>    // for assert
#include <string.h>    // for strcpy, strlen
#include <string>      // for allocator, string
int SqlParse(const char *st, Query *&res);
int Parse(const char *st, Query *&res) {
    SqlParse(st, res);
    if (res->GetScf() == ScfError) {
        return 0;
    }
    return 1;
}
int *InitIntsValue(int value) {
    int *res = new int;
    *res = value;
    return res;
}
float *InitFloatValue(float value) {
    float *res = new float;
    *res = value;
    return res;
}
char *InitCharsValue(const char *value) {
    char *res = StrNew(value);
    return res;
}
// parse_defs
//----------------------------------------------------
char *StrNew(const char *str) {
    int len = int(strlen(str));
    char *res = new char[len + 1];
    strcpy(res, str);
    res[len] = '\0';
    return res;
}
const char *StrAttrType(AttrType type) {
    switch (type) {
        case AttrType::Undefined:
            return "Undefined";
        case AttrType::Ints:
            return "Ints";
        case AttrType::Floats:
            return "Floats";
        case AttrType::Chars:
            return "Chars";
        case AttrType::Dates:
            return "Dates";
        default:
            assert(false);
    }
}
const char *StrCompOp(CompOp cmp) {
    switch (cmp) {
        case CompOp::NoOp:
            return "NoOp";
        case CompOp::EqualTo:
            return "EqualTo";
        case CompOp::GreatEqual:
            return "GreatEqual";
        case CompOp::GreatThan:
            return "GreatThan";
        case CompOp::LessEqual:
            return "LessEqual";
        case CompOp::LessThan:
            return "LessThan";
        case CompOp::NotEqual:
            return "NotEqual";
        default:
            assert(false);
    }
}