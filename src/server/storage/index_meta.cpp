#include "index_meta.h"
#include <jsoncpp/json/json.h>
#include "../../common/common_defs.h"               // for DebugPrint
#include "table.h"
#include "field.h"
const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");
Re IndexMeta::Init(const char *name, const FieldMeta &field) {
    if (StrBlank(name)) {
        DebugPrint("IndexMeta:Failed to init index, index_name is empty.\n");
        return Re::InvalidArgument;
    }
    index_name_ = name, field_name_ = field.GetFieldName();
    return Re::Success;
}
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