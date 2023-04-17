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

std::string strAttrType(AttrType type) {
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

std::string strCompOp(CompOp cmp) {
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