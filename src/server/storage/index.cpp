#include "index.h"

#include <jsoncpp/json/config.h>                      // for String
#include <jsoncpp/json/value.h>                       // for Value, StaticSt...
#include <cstring>                                    // for strlen

#include "../../common/common_defs.h"                 // for DebugPrint
#include "field.h"                                    // for FieldMeta
#include "table.h"                                    // for TableMeta
#include "/home/ubuntu/idbms/src/server/common/re.h"  // for Re, GenericError

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");
Re IndexMeta::Init(const char *name, const FieldMeta &field) {
    if (strlen(name) == 0) {
        DebugPrint("IndexMeta:Failed to init index, index_name is empty.\n");
        return Re::InvalidArgument;
    }
    index_name_ = name, field_name_ = field.GetFieldName();
    return Re::Success;
}
std::string IndexMeta::GetIndexName() const { return index_name_; }
std::string IndexMeta::GetFieldName() const { return field_name_; }
void IndexMeta::ToJson(Json::Value &json_value) const {
    json_value[FIELD_NAME] = index_name_;
    json_value[FIELD_FIELD_NAME] = field_name_;
}
Re IndexMeta::FromJson(const TableMeta &table_meta, const Json::Value &json_value, IndexMeta &index_meta) {
    const Json::Value &name_value = json_value[FIELD_NAME];
    const Json::Value &field_value = json_value[FIELD_FIELD_NAME];
    if (!name_value.isString()) {
        DebugPrint("IndexMeta:index name is not a string. json value=%s\n", name_value.toStyledString().c_str());
        return Re::GenericError;
    }
    if (!field_value.isString()) {
        DebugPrint("IndexMeta:field name of index [%s] is not a string. json value=%s\n", name_value.asCString(),
                   field_value.toStyledString().c_str());
        return Re::GenericError;
    }
    const FieldMeta *field = table_meta.GetField(field_value.asCString());
    if (field == nullptr) {
        DebugPrint("IndexMeta:deserialize index [%s]: no such field: %s\n", name_value.asCString(),
                   field_value.asCString());
        return Re::SchemaFieldMissing;
    }
    return index_meta.Init(name_value.asCString(), *field);
}