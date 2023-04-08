#pragma once

#include <cstring>
#include <string>
#include "../common/server_defs.h"
#include "../common/re.h"

class FieldMeta;

class TableMeta;

namespace Json {
    class Value;
}  // namespace Json

class IndexMeta {
public:
    IndexMeta() = default;

    Re init(const char *name, const FieldMeta &field);

    [[nodiscard]] std::string getIndexName() const;

    [[nodiscard]] std::string getFieldName() const;

//    void desc(std::ostream &os) const;
    void toJson(Json::Value &json_value) const;

    static Re fromJson(const TableMeta &table_meta, const Json::Value &json_value, IndexMeta &index_meta);

private:
    std::string index_name_, field_name_;
};

class Index {

};