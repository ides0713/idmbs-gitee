#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <cstring>
#include <string>
#include "../common/re.h"

namespace Json {
    class Value;
}

class FieldMeta {
public:
    FieldMeta() : attr_type_(AttrType::Undefined), offset_(-1), len_(0), visible_(false) {}

    Re initialize(const char *field_name, AttrType attr_type, int attr_offset, int attr_len, bool visible);

    [[nodiscard]] std::string getFieldName() const { return field_name_; }

    [[nodiscard]] AttrType getAttrType() const { return attr_type_; }

    [[nodiscard]] int getOffset() const { return offset_; }

    [[nodiscard]] int getLen() const { return len_; }

    [[nodiscard]] bool getVisible() const { return visible_; }

    void toJson(Json::Value &json_value) const;

public:
    static Re fromJson(const Json::Value &json_value, FieldMeta &field);

private:
    std::string field_name_;
    AttrType attr_type_;
    int offset_, len_;
    bool visible_;
};

class Table;

class Field {
public:
    Field() : table_(nullptr), field_meta_(nullptr) {}

    Field(Table *table, FieldMeta *field) : table_(table), field_meta_(field) {}

    [[nodiscard]] const Table *getTable() const { return table_; }

    [[nodiscard]] const FieldMeta *getFieldMeta() const { return field_meta_; }

    void setTable(Table *table) { this->table_ = table; }

    void setFieldMeta(FieldMeta *field) { this->field_meta_ = field; }

    [[nodiscard]] AttrType getAttrType() const { return field_meta_->getAttrType(); }

    [[nodiscard]] std::string getTableName() const;

    [[nodiscard]] std::string getFieldName() const { return field_meta_->getFieldName(); }

private:
    Table *table_;
    FieldMeta *field_meta_;
};