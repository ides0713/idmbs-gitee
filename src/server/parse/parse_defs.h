#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>

const int MAX_ID_LENGTH = 20;
const int MAX_REL_NAME_LENGTH = 20;
const int MAX_RELS_NUM = 20;
const int MAX_ATTRS_NUM = 20;
const int MAX_CONDITIONS_NUM = 20;
const int MAX_ATTR_LENGTH = 20;
const int MAX_MSG_LENGTH = 50;
const int MAX_VALUES_NUM = 20;

char *StrNew(const char *str);

int *InitIntsValue(int value);

float *InitFloatValue(float value);

char *InitCharsValue(const char *value);

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

std::string StrAttrType(AttrType type);

enum CompOp {
    EqualTo = 0,//"="     0
    LessEqual,  //"<="    1
    NotEqual,   //"<>"    2
    LessThan,   //"<"     3
    GreatEqual, //">="    4
    GreatThan,  //">"     5
    NoOp
};

std::string StrCompOp(CompOp cmp);

struct Value {
public:
    AttrType type;// 属性类型(数据类型)
    void *data;   // 数据内容(值)
public:
    ///@brief debug
    void Desc(std::ostream &stream) const {
        stream << "[type: " << StrAttrType(type) << " data: ";
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

    void Copy(const Value &value) {
        if (data != nullptr) {
            Destroy();
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
                data = StrNew(reinterpret_cast<char *>(value.data));
                break;
            case AttrType::Dates:
                assert(false);
                break;
        }
    }

    void Destroy() {
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
    char *rel_name; // 关系名(表名)
    char *attr_name;// 属性名
public:
    void Desc(std::ostream &stream) const {
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
        attr_name = StrNew(a_name);
    }

    void Copy(const RelAttr &attr) {
        Destroy();
        if (attr.rel_name != nullptr)
            rel_name = StrNew(attr.rel_name);
        if (attr.attr_name != nullptr)
            attr_name = StrNew(attr.attr_name);
    }

    void Destroy() {
        delete[] rel_name;
        delete[] attr_name;
    }
};

struct AttrInfo {
public:
    char *attr_name;   // 属性名
    AttrType attr_type;// 属性类型(数据类型)
    size_t attr_len;   // 属性长度(占空间大小)
public:
    void Desc(std::ostream &stream) const {
        stream << "[attr_name: " << attr_name << " attr_type: " << StrAttrType(attr_type) << " attr_len: " << attr_len
               << "]";
    }

    AttrInfo() {
        attr_name = nullptr;
        attr_type = AttrType::Undefined;
        attr_len = 0;
    }

    AttrInfo(const char *name, AttrType type, size_t len = 1) {
        attr_name = StrNew(name);
        attr_type = type;
        attr_len = len;
    }

    AttrInfo(const AttrInfo &attr_info) {
        attr_name = StrNew(attr_info.attr_name);
        attr_type = attr_info.attr_type;
        attr_len = attr_info.attr_len;
    }

    void Copy(const AttrInfo &attr_info) {
        attr_name = StrNew(attr_info.attr_name);
        attr_type = attr_info.attr_type;
        attr_len = attr_info.attr_len;
    }

    void Destroy() const {
        delete[] attr_name;
    }

    AttrInfo &operator=(const AttrInfo &attr_info) {
        if (this == &attr_info)
            return *this;
        attr_name = StrNew(attr_info.attr_name);
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
    void Desc(std::ostream &stream) const {
        stream << "LEFT ";
        if (left_is_attr) {
            stream << "RelAttr: ";
            left_attr.Desc(stream);
        } else {
            stream << "Value: ";
            left_value.Desc(stream);
        }
        stream << '\n'
               << "RIGHT ";
        if (right_is_attr) {
            stream << "RelAttr: ";
            right_attr.Desc(stream);
        } else {
            stream << "Value: ";
            right_value.Desc(stream);
        }
    }

    void Init(CompOp c, int l_is_attr, RelAttr *l_attr, Value *l_value,
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

    void Copy(const Condition &c) {
        comp = c.comp;
        left_is_attr = c.left_is_attr, right_is_attr = c.right_is_attr;
        if (left_is_attr)
            left_attr.Copy(c.left_attr);
        else
            left_value.Copy(c.left_value);
        if (right_is_attr)
            right_attr.Copy(c.right_attr);
        else
            right_value.Copy(c.right_value);
    }

    void Destroy() {
        if (left_is_attr)
            left_attr.Destroy();
        else
            left_value.Destroy();
        if (right_is_attr)
            right_attr.Destroy();
        else
            right_value.Destroy();
    }
};

class Query {
public:
    Query() : flag_(ScfError) {}

    explicit Query(SqlCommandFlag flag) : flag_(flag) {}

    virtual void Init() = 0;

    //~query()
    virtual void Destroy() = 0;

    SqlCommandFlag GetScf() { return flag_; }

private:
    SqlCommandFlag flag_;
};

class SelectQuery : public Query {
public:
    SelectQuery() : Query(SqlCommandFlag::ScfSelect), attrs_(nullptr), rel_names_(nullptr), conditions_(nullptr) {}

    void Init() override {
        attrs_num_ = 0, rel_names_num_ = 0, conditions_num_ = 0;
        attrs_ = new RelAttr[MAX_ATTRS_NUM];
        conditions_ = new Condition[MAX_CONDITIONS_NUM];
        rel_names_ = new char *[MAX_RELS_NUM];
    }

    void Destroy() override {
        for (int i = 0; i < rel_names_num_; i++)
            delete[] rel_names_[i];
        delete[] rel_names_;
        for (int i = 0; i < attrs_num_; i++)
            attrs_[i].Destroy();
        delete[] attrs_;
        for (int i = 0; i < conditions_num_; i++)
            conditions_[i].Destroy();
        delete[] conditions_;
    }

    void AddRelAttr(const RelAttr &rel_attr) { attrs_[attrs_num_++] = rel_attr; }

    void AddRelName(const char *str) { rel_names_[rel_names_num_++] = StrNew(str); }

    void AddConditions(size_t conditions_num, const Condition *conditions) {
        for (int i = 0; i < conditions_num; i++)
            conditions_[conditions_num_++] = conditions[i];
    }

    int GetAttrsNum() { return attrs_num_; }

    int GetRelNamesNum() { return rel_names_num_; }

    int GetConditionsNum() { return conditions_num_; }

    RelAttr *GetAttrs() { return attrs_; }

    Condition *GetConditions() { return conditions_; }

    char **GetRelNames() { return rel_names_; }

private:
    int attrs_num_, rel_names_num_, conditions_num_;
    RelAttr *attrs_;
    char **rel_names_;
    Condition *conditions_;
};

class InsertQuery : public Query {
public:
    InsertQuery() : Query(SqlCommandFlag::ScfInsert), rel_name_(nullptr), values_(nullptr) {}

    void Init() override {
        rel_name_ = nullptr;
        values_num_ = 0;
        values_ = new Value[MAX_VALUES_NUM];
    }

    void Destroy() override {
        delete[] rel_name_;
        for (int i = 0; i < values_num_; i++)
            values_[i].Destroy();
        delete[] values_;
    }

    void SetRelName(const char *str) { rel_name_ = StrNew(str); }

    void AddValue(const Value &value) { values_[values_num_++] = value; }

    void AddValues(const size_t value_num, const Value *values) {
        for (int i = 0; i < value_num; i++)
            values_[values_num_++] = values[i];
    }

    char *GetRelName() { return rel_name_; }

    [[nodiscard]] int GetValuesNum() const { return values_num_; }

    Value *GetValues() { return values_; }

private:
    char *rel_name_;
    int values_num_;
    Value *values_;
};

class CreateTableQuery : public Query {
public:
    CreateTableQuery() : Query(SqlCommandFlag::ScfCreateTable), rel_name_(nullptr), attrs_num_(0), attrs_(nullptr) {}

    void Init() override {
        rel_name_ = nullptr;
        attrs_num_ = 0;
        attrs_ = new AttrInfo[MAX_ATTRS_NUM];
    }

    void Destroy() override {
        delete[] rel_name_;
        for (int i = 0; i < attrs_num_; i++)
            attrs_[i].Destroy();
    }

    void SetRelName(const char *str) { rel_name_ = StrNew(str); }

    void AddAttr(const AttrInfo &attr) { attrs_[attrs_num_++] = attr; }

    char *GetRelName() { return rel_name_; }

    [[nodiscard]] int GetAttrNum() const { return attrs_num_; }

    AttrInfo *GetAttrs() { return attrs_; }

private:
    char *rel_name_;
    int attrs_num_;
    AttrInfo *attrs_;
};

class DeleteQuery : public Query {
public:
    DeleteQuery() : Query(SqlCommandFlag::ScfDelete), rel_name_(nullptr), conditions_(nullptr) {}
    void Init() override {
        rel_name_ = nullptr;
        conditions_num_ = 0;
    }
    void Destroy() override;
    void SetRelName(const char *str) { rel_name_ = StrNew(str); }

private:
    int conditions_num_;
    char *rel_name_;
    Condition *conditions_;
    int abcd_;
};
class ErrorQuery : public Query {
public:
    explicit ErrorQuery(const char *str) : Query(SqlCommandFlag::ScfError) {
        error_message_ = new char[MAX_MSG_LENGTH];
        strcpy(error_message_, str);
    }

    void Init() override {}

    void Destroy() override { delete[] error_message_; }

private:
    char *error_message_;
};
