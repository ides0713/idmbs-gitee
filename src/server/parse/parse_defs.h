#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>

const int MAX_ID_LENGTH = 20;
const int MAX_REL_NAME_LENGTH = 20;
const int MAX_RELS_NUM = 20;
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

std::string strAttrType(AttrType type);

enum CompOp {
    EqualTo = 0, //"="     0
    LessEqual,   //"<="    1
    NotEqual,    //"<>"    2
    LessThan,    //"<"     3
    GreatEqual,  //">="    4
    GreatThan,   //">"     5
    NoOp
};

std::string strCompOp(CompOp cmp);

struct Value {
public:
    AttrType type; // 属性类型(数据类型)
    void *data;    // 数据内容(值)
public:
    ///@brief debug
    void desc(std::ostream &stream) const {
        stream << "[type: " << strAttrType(type) << " data: ";
        switch (type) {
            case AttrType::Ints:
                stream << *reinterpret_cast<int *>(data);
                break;
            case AttrType::Floats:
                stream << *reinterpret_cast<float *>(data);
                break;
            case AttrType::Chars:
                stream << reinterpret_cast<char *>(data);
                break;
            case AttrType::Undefined:
            case AttrType::Dates:
            default:
                assert(false);
        }
        stream << "]";
    }

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
                data = strNew(reinterpret_cast<char *>(value.data));
                break;
            case AttrType::Dates:
                assert(false);
                break;
        }
    }

    void destroy() {
        if (data == nullptr)
            return;
        switch (type) {
            case AttrType::Undefined:
                break;
            case AttrType::Ints:
                delete reinterpret_cast<int *>(data);
                break;
            case AttrType::Floats:
                delete reinterpret_cast<float *>(data);
                break;
            case AttrType::Chars:
                delete reinterpret_cast<char *>(data);
                break;
            case AttrType::Dates:
                break;
        }
        data = nullptr;
    }
};

struct RelAttr {
public:
    char *rel_name;  // 关系名(表名)
    char *attr_name; // 属性名
public:
    void desc(std::ostream &stream) const {
        stream << "[rel_name: ";
        if (rel_name == nullptr)
            stream << "null";
        else
            stream << rel_name;
        stream << " attr_name: " << attr_name << "]";
    }

    RelAttr() : rel_name(nullptr), attr_name(nullptr) {}

    RelAttr(const char *r_name, const char *a_name) {
        if (r_name == nullptr)
            rel_name = nullptr;
        attr_name = strNew(a_name);
    }

    void copy(const RelAttr &attr) {
        destroy();
        if (attr.rel_name != nullptr)
            rel_name = strNew(attr.rel_name);
        if (attr.attr_name != nullptr)
            attr_name = strNew(attr.attr_name);
    }

    void destroy() {
        delete[] rel_name;
        delete[] attr_name;
    }
};

struct AttrInfo {
public:
    char *attr_name;    // 属性名
    AttrType attr_type; // 属性类型(数据类型)
    size_t attr_len;    // 属性长度(占空间大小)
public:
    void desc(std::ostream &stream) const {
        stream << "[attr_name: " << attr_name << " attr_type: " << strAttrType(attr_type) << " attr_len: " << attr_len
               << "]";
    }

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

    void copy(const AttrInfo &attr_info) {
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
public:
    // *is_attr 用于标识比较符两侧是否为属性名(可以为具体值)
    int left_is_attr;
    // 显然 *value 与 *attr仅能使用一个 (如 左侧为具体值 则 left_attr 无意义 反之则 left_value 无意义)
    Value left_value;
    RelAttr left_attr;
    CompOp comp;
    int right_is_attr;
    RelAttr right_attr;
    Value right_value;
public:
    void desc(std::ostream &stream) const {
        stream << "LEFT ";
        if (left_is_attr) {
            stream << "RelAttr: ";
            left_attr.desc(stream);
        } else {
            stream << "Value: ";
            left_value.desc(stream);
        }
        stream << '\n' << "RIGHT ";
        if (right_is_attr) {
            stream << "RelAttr: ";
            right_attr.desc(stream);
        } else {
            stream << "Value: ";
            right_value.desc(stream);
        }
    }

    void init(CompOp c, int l_is_attr, RelAttr *l_attr, Value *l_value,
              int r_is_attr, RelAttr *r_attr, Value *r_value) {
        comp = c;
        left_is_attr = l_is_attr;
        if (l_is_attr)
            left_attr = *l_attr;
        else
            left_value = *l_value;
        right_is_attr = r_is_attr;
        if (right_is_attr)
            right_attr = *r_attr;
        else
            right_value = *r_value;
    }

    void copy(const Condition &c) {
        comp = c.comp;
        left_is_attr = c.left_is_attr, right_is_attr = c.right_is_attr;
        if (left_is_attr)
            left_attr.copy(c.left_attr);
        else
            left_value.copy(c.left_value);
        if (right_is_attr)
            right_attr.copy(c.right_attr);
        else
            right_value.copy(c.right_value);
    }

    void destroy() {
        if (left_is_attr)
            left_attr.destroy();
        else
            left_value.destroy();
        if (right_is_attr)
            right_attr.destroy();
        else
            right_value.destroy();
    }
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
    SelectQuery() : Query(ScfSelect), attrs_(nullptr), rel_names_(nullptr), conditions_(nullptr) {}

    void init() override {
        attrs_num_ = 0, rel_names_num_ = 0, conditions_num_ = 0;
        attrs_ = new RelAttr[MAX_ATTRS_NUM];
        conditions_ = new Condition[MAX_CONDITIONS_NUM];
        rel_names_ = new char *[MAX_RELS_NUM];
    }

    void destroy() override {
        for (int i = 0; i < rel_names_num_; i++)
            delete[]rel_names_[i];
        delete[]rel_names_;
        for (int i = 0; i < attrs_num_; i++)
            attrs_[i].destroy();
        delete[]attrs_;
        for (int i = 0; i < conditions_num_; i++)
            conditions_[i].destroy();
        delete[]conditions_;
    }

    void addRelAttr(const RelAttr &rel_attr) { attrs_[attrs_num_++] = rel_attr; }

    void addRelName(const char *str) { rel_names_[rel_names_num_++] = strNew(str); }

    void addConditions(size_t conditions_num, const Condition *conditions) {
        for (int i = 0; i < conditions_num; i++)
            conditions_[conditions_num_++] = conditions[i];
    }

    int getAttrsNum() { return attrs_num_; }

    int getRelNamesNum() { return rel_names_num_; }

    int getConditionsNum() { return conditions_num_; }

    RelAttr *getAttrs() { return attrs_; }

    Condition *getConditions() { return conditions_; }

    char **getRelNames() { return rel_names_; }

private:
    int attrs_num_, rel_names_num_, conditions_num_;
    RelAttr *attrs_;
    char **rel_names_;
    Condition *conditions_;
};

class InsertQuery : public Query {
public:
    InsertQuery() : Query(ScfInsert), rel_name_(nullptr), values_(nullptr) {}

    void init() override {
        rel_name_ = nullptr;
        values_num_ = 0;
        values_ = new Value[MAX_VALUES_NUM];
    }

    void destroy() override {
        delete[] rel_name_;
        for (int i = 0; i < values_num_; i++)
            values_[i].destroy();
        delete[]values_;
    }

    void setRelName(const char *str) { rel_name_ = strNew(str); }

    void addValue(const Value &value) { values_[values_num_++] = value; }

    void addValues(const size_t value_num, const Value *values) {
        for (int i = 0; i < value_num; i++)
            values_[values_num_++] = values[i];
    }

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
