#pragma once
#include "../common/re.h"
#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <cstring>
#include <string>
namespace Json {
    class Value;
}
class FieldMeta
{
public:
    FieldMeta() : attr_type_(AttrType::Undefined), offset_(-1), len_(0), visible_(false) {}
    Re Init(const char *field_name, AttrType attr_type, int attr_offset, int attr_len, bool visible);
    [[nodiscard]] const char *GetFieldName() const { return field_name_.c_str(); }
    [[nodiscard]] AttrType GetAttrType() const { return attr_type_; }
    [[nodiscard]] int GetOffset() const { return offset_; }
    [[nodiscard]] int GetLen() const { return len_; }
    [[nodiscard]] bool GetVisible() const { return visible_; }
    void ToJson(Json::Value &json_value) const;

public:
    static Re FromJson(const Json::Value &json_value, FieldMeta &field);

private:
    std::string field_name_;
    AttrType attr_type_;
    int offset_, len_;
    bool visible_;
};
class Table;
///@brief class Field is just a container of pointer of field meta and pointer of table,it does not have its own memeber
class Field
{
public:
    Field() : table_(nullptr), field_meta_(nullptr) {}
    Field(const Table *table, const FieldMeta *field) : table_(table), field_meta_(field) {}
    [[nodiscard]] const Table *GetTable() const { return table_; }
    [[nodiscard]] const FieldMeta *GetFieldMeta() const { return field_meta_; }
    void SetTable(Table *table) { this->table_ = table; }
    void SetFieldMeta(FieldMeta *field) { this->field_meta_ = field; }
    [[nodiscard]] AttrType GetAttrType() const { return field_meta_->GetAttrType(); }
    [[nodiscard]] const char *GetTableName() const;
    [[nodiscard]] const char *GetFieldName() const { return field_meta_->GetFieldName(); }

private:
    const Table *table_;
    const FieldMeta *field_meta_;
};