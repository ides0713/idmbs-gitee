#pragma once
#include <string>          // for string

#include "../common/re.h"  // for Re

class FieldMeta;
class TableMeta;

namespace Json {
    class Value;
}
class IndexMeta
{
public:
    IndexMeta() = default;
    Re Init(const char *name, const FieldMeta &field);
    [[nodiscard]] std::string GetIndexName() const;
    [[nodiscard]] std::string GetFieldName() const;
    //    void desc(std::ostream &os) const;
    void ToJson(Json::Value &json_value) const;
    static Re FromJson(const TableMeta &table_meta, const Json::Value &json_value, IndexMeta &index_meta);

private:
    std::string index_name_, field_name_;
};
class Index
{
};