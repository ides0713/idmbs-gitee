#include "parse.h"

#include <assert.h>      // for assert
#include <string.h>      // for strcpy, strlen
#include <string>        // for allocator, string

#include "parse_defs.h"  // for CompOp, AttrType, Chars, Dates, EqualTo, Floats

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
std::string StrAttrType(AttrType type) {
    switch (type) {
        case AttrType::Undefined:
            return std::string{"Undefined"};
        case AttrType::Ints:
            return std::string{"Ints"};
        case AttrType::Floats:
            return std::string{"Floats"};
        case AttrType::Chars:
            return std::string{"Chars"};
        case AttrType::Dates:
            return std::string{"Dates"};
        default:
            assert(false);
    }
}
std::string StrCompOp(CompOp cmp) {
    switch (cmp) {
        case CompOp::NoOp:
            return std::string{"NoOp"};
        case CompOp::EqualTo:
            return std::string{"EqualTo"};
        case CompOp::GreatEqual:
            return std::string{"GreatEqual"};
        case CompOp::GreatThan:
            return std::string{"GreatThan"};
        case CompOp::LessEqual:
            return std::string{"LessEqual"};
        case CompOp::LessThan:
            return std::string{"LessThan"};
        case CompOp::NotEqual:
            return std::string{"NotEqual"};
        default:
            assert(false);
    }
}