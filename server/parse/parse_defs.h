#pragma once
#include <stdio.h>
#include <string.h>

const int MAX_ID_LENGTH=20;
const int MAX_REL_LENGTH = 20;
const int MAX_ATTRS_NUM = 20;
const int MAX_CONDITIONS_NUM = 20;
const int MAX_ATTR_LENGTH = 20;
const int MAX_MSG_LENGTH = 50;

// add func
int test_func(int param);
char * strnew(const char * str);

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
    AttrInfo(){
        attr_name=nullptr;
    }
    AttrInfo(const char *name,AttrType type,size_t len=1){
        attr_name=strnew(name);
        attr_type=type;
        attr_len=len;
    }
    void destroy(){
        delete[]attr_name;
    }
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

class Query{
    public:
    Query():flag_(SCF_ERROR){}
    Query(SqlCommandFlag flag):flag_(flag){}
    virtual void initialize()=0;
    //~xxxquery()
    virtual void destroy()=0;
    SqlCommandFlag getSCF(){return flag_;}
    private:
    SqlCommandFlag flag_;
};


class SelectQuery:public Query
{
    public:
    SelectQuery():Query(SCF_SELECT){
        rel_name_=nullptr;
    }
    void initialize() override{
        rel_name_=new char [MAX_REL_LENGTH+1];
    }
    void destroy() override{
        delete[]rel_name_;
    }
    private:
    char *rel_name_;
};
class InsertQuery:public Query
{
    public:
    InsertQuery():Query(SCF_INSERT){
        rel_name_=nullptr;
    }
    void initialize() override{
        rel_name_=new char [MAX_REL_LENGTH+1];
    }
    void destroy() override{
        delete[] rel_name_;
    }
    private:
    char *rel_name_;
};
class CreateTableQuery:public Query
{
    public:
    CreateTableQuery():Query(SCF_CREATE_TABLE){}
    void initialize() override{
        rel_name_=nullptr;
        attr_num_=0;
        attrs_=new AttrInfo[MAX_ATTRS_NUM];
    }
    void destroy() override{
        delete[]rel_name_;
        for(int i=0;i<attr_num_;i++)
            attrs_[i].destroy();
    }
    void setRelName(const char * str){
        rel_name_=strnew(str);
    }
    void addAttr(const AttrInfo& attr){
        attrs_[attr_num_++]=attr;
    }
    private:
    char * rel_name_;
    size_t attr_num_;
    AttrInfo* attrs_;
};

class ErrorQuery:public Query
{
    public:
    ErrorQuery(const char * str):Query(SCF_ERROR){
        error_message_=new char [MAX_MSG_LENGTH];
        strcpy(error_message_,str);
    }
    void initialize(){}
    void destroy(){
        delete [] error_message_;
    }
    private:
    char * error_message_;
};
