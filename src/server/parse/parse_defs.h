#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>

const int MAX_ID_LENGTH = 20;
const int MAX_REL_LENGTH = 20;
const int MAX_ATTRS_NUM = 20;
const int MAX_CONDITIONS_NUM = 20;
const int MAX_ATTR_LENGTH = 20;
const int MAX_MSG_LENGTH = 50;
const int MAX_VALUES_NUM = 20;

char *strNew(const char *str);

int *initIntsValue(int value);

float *initFloatValue(float value);

char *initCharsValue(const char *value);

enum SqlCommandFlag {
    // SCF_ERROR 解析失败
    ScfError = 0,
    ScfSelect,
    ScfInsert,
    ScfUpdate,
    ScfDelete,
    ScfCreateTable,
    ScfDropTable,
    ScfCreateIndex,
    ScfDropIndex,
    ScfSync,
    ScfShowTables,
    ScfDescTable,
    ScfBegin,
    ScfCommit,
    ScfClogSync,
    ScfRollback,
    ScfLoadData,
    ScfHelp,
    ScfExit
};

enum AttrType {
    Undefined = 0,
    Chars,
    Ints,
    Floats,
    Dates
};

enum CompOp {
    EqualTo = 0, //"="     0
    LessEqual,   //"<="    1
    NotEqual,    //"<>"    2
    LessThan,    //"<"     3
    GreatEqual,  //">="    4
    GreatThan,   //">"     5
    NoOp
};

struct Value {
public:
    AttrType type; // 属性类型(数据类型)
    void *data;    // 数据内容(值)
public:
    Value() : type(AttrType::Undefined), data(nullptr) {}

    Value(AttrType t, void *d) : type(t), data(d) {}

    void copy(const Value &value) {
        if (data != nullptr) {
            destroy();
            data = nullptr;
        }
        switch (value.type) {
            case AttrType::Undefined:
                type = AttrType::Undefined;
                data = nullptr;
                break;
            case AttrType::Ints:
                type = AttrType::Ints;
                data = new int;
                memcpy(data, value.data, sizeof(int));
                break;
            case AttrType::Floats:
                type = AttrType::Floats;
                data = new float;
                memcpy(data, value.data, sizeof(float));
                break;
            case AttrType::Chars:
                type = AttrType::Chars;
                data = strNew(static_cast<char *>(value.data));
                break;
            case AttrType::Dates:
                assert(false);
                break;
        }
    }

    void destroy() {
        if (data = nullptr)
            return;
        switch (type) {
            case AttrType::Undefined:
                break;
            case AttrType::Ints:
                delete static_cast<int *>(data);
                break;
            case AttrType::Floats:
                delete static_cast<float *>(data);
                break;
            case AttrType::Chars:
                delete static_cast<char *>(data);
                break;
            case AttrType::Dates:
                break;
        }
        data = nullptr;
    }
};

struct RelAttr {
    char *rel_name;  // 关系名(表名)
    char *attr_name; // 属性名
};

struct AttrInfo {
    char *attr_name;    // 属性名
    AttrType attr_type; // 属性类型(数据类型)
    size_t attr_len;    // 属性长度(占空间大小)
    AttrInfo() {
        attr_name = nullptr;
        attr_type = Undefined;
        attr_len = 0;
    }

    AttrInfo(const char *name, AttrType type, size_t len = 1) {
        attr_name = strNew(name);
        attr_type = type;
        attr_len = len;
    }

    AttrInfo(const AttrInfo &attr_info) {
        attr_name = strNew(attr_info.attr_name);
        attr_type = attr_info.attr_type;
        attr_len = attr_info.attr_len;
    }

    void destroy() const {
        delete[] attr_name;
    }

    AttrInfo &operator=(const AttrInfo &attr_info) {
        if (this == &attr_info) return *this;
        attr_name = strNew(attr_info.attr_name);
        attr_type = attr_info.attr_type;
        attr_len = attr_info.attr_len;
        return *this;
    }
};

struct Condition {
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

class Query {
public:
    Query() : flag_(ScfError) {}

    explicit Query(SqlCommandFlag flag) : flag_(flag) {}

    virtual void init() = 0;

    //~query()
    virtual void destroy() = 0;

    SqlCommandFlag getScf() { return flag_; }

private:
    SqlCommandFlag flag_;
};

class SelectQuery : public Query {
public:
    SelectQuery() : Query(ScfSelect) { rel_name_ = nullptr; }

    void init() override { rel_name_ = new char[MAX_REL_LENGTH + 1]; }

    void destroy() override { delete[] rel_name_; }

private:
    char *rel_name_;
};

class InsertQuery : public Query {
public:
    InsertQuery() : Query(ScfInsert) { rel_name_ = nullptr; }

    void init() override {
        rel_name_ = nullptr;
        values_num_ = 0;
        values_ = new Value[MAX_VALUES_NUM];
    }

    void destroy() override {
        delete[] rel_name_;
        for (int i = 0; i < values_num_; i++)
            values_[i].destroy();
    }

    void setRelName(const char *str) { rel_name_ = strNew(str); }

    void addValue(const Value &value) { values_[values_num_++] = value; }

    char *getRelName() { return rel_name_; }

    [[nodiscard]]int getValuesNum() const { return values_num_; }

    Value *getValues() { return values_; }

private:
    char *rel_name_;
    int values_num_;
    Value *values_;
};

class CreateTableQuery : public Query {
public:
    CreateTableQuery() : Query(ScfCreateTable) { rel_name_ = nullptr, attrs_num_ = 0, attrs_ = nullptr; }

    void init() override {
        rel_name_ = nullptr;
        attrs_num_ = 0;
        attrs_ = new AttrInfo[MAX_ATTRS_NUM];
    }

    void destroy() override {
        delete[] rel_name_;
        for (int i = 0; i < attrs_num_; i++)
            attrs_[i].destroy();
    }

    void setRelName(const char *str) { rel_name_ = strNew(str); }

    void addAttr(const AttrInfo &attr) { attrs_[attrs_num_++] = attr; }

    char *getRelName() { return rel_name_; }

    [[nodiscard]] int getAttrNum() const { return attrs_num_; }

    AttrInfo *getAttrs() { return attrs_; }

private:
    char *rel_name_;
    int attrs_num_;
    AttrInfo *attrs_;
};

class ErrorQuery : public Query {
public:
    explicit ErrorQuery(const char *str) : Query(ScfError) {
        error_message_ = new char[MAX_MSG_LENGTH];
        strcpy(error_message_, str);
    }

    void init() override {}

    void destroy() override { delete[] error_message_; }

private:
    char *error_message_;
};
