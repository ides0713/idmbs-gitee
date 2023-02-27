#pragma once
#include <stdio.h>

const int MAX_ID_LENGTH = 20;
const int MAX_ATTRS_NUM = 20;
const int MAX_CONDITIONS_NUM = 20;
const int MAX_ATTR_LENGTH = 20;

enum SqlCommandFlag
{
    // SCF_ERROR 解析失败
    SCF_ERROR = 0,
    SCF_SELECT,
    SCF_INSERT,
    SCF_UPDATE,
    SCF_DELETE,
    SCF_CREATE_TABLE,
    SCF_DROP_TABLE,
    SCF_CREATE_INDEX,
    SCF_DROP_INDEX,
    SCF_SYNC,
    SCF_SHOW_TABLES,
    SCF_DESC_TABLE,
    SCF_BEGIN,
    SCF_COMMIT,
    SCF_CLOG_SYNC,
    SCF_ROLLBACK,
    SCF_LOAD_DATA,
    SCF_HELP,
    SCF_EXIT
};

enum AttrType
{
    UNDEFINED = 0,
    CHARS,
    INTS,
    FLOATS,
    DATES
};

enum CompOp
{
    EQUAL_TO = 0, //"="     0
    LESS_EQUAL,   //"<="    1
    NOT_EQUAL,    //"<>"    2
    LESS_THAN,    //"<"     3
    GREAT_EQUAL,  //">="    4
    GREAT_THAN,   //">"     5
    NO_OP
};

struct Value
{
    AttrType type; // 属性类型(数据类型)
    void *data;    // 数据内容(值)
};

struct RelAttr
{
    char *rel_name;  // 关系名(表名)
    char *attr_name; // 属性名
};
struct AttrInfo
{
    char *attr_name;    // 属性名
    AttrType attr_type; // 属性类型(数据类型)
    size_t attr_len;    // 属性长度(占空间大小)
};

struct Condition
{
    // *is_attr 用于标识比较符两侧是否为属性名(可以为具体值)
    int left_is_attr;
    // 显然 *value 与 *attr仅能使用一个 (如 左侧为具体值 则 left_attr 无意义 反之则 left_value 无意义)
    Value left_value;
    RelAttr left_attr;
    CompOp comp;
    int right_is_attr;
    RelAttr right_attr;
    Value right_value;
};

struct SelectQuery
{
    char *rel_name;
};
struct InsertQuery
{
    char *rel_name;
};
//Query中的值是parse阶段中获得信息的集合
union Query
{
    SelectQuery select_query;
    InsertQuery insert_query;
    char *error_info;
};

struct QueryInfo{
    SqlCommandFlag SCF_Flag;
    Query query;
};

// add func
int test_func(int param);