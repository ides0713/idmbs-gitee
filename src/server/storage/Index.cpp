#include "Index.h"
#include "../../common/common_defs.h"
#include "Field.h"
#include <jsoncpp/json/json.h>
#include "table.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_FIELD_NAME("field_name");

Re IndexMeta::initialize(const char *name, const FieldMeta &field) {
    if (strlen(name) == 0) {
        debugPrint("IndexMeta:Failed to init index, index_name is empty.\n");
        return Re::InvalidArgument;
    }
    index_name_ = name, field_name_ = field.getFieldName();
    return Re::Success;
}

std::string IndexMeta::getIndexName() const {
    return index_name_;
}

std::string IndexMeta::getFieldName() const {
    return field_name_;
}

void IndexMeta::toJson(Json::Value &json_value) const {
    json_value[FIELD_NAME] = index_name_;
    json_value[FIELD_FIELD_NAME] = field_name_;
}

Re IndexMeta::fromJson(const TableMeta &table_meta, const Json::Value &json_value, IndexMeta &index_meta) {
    const Json::Value &name_value = json_value[FIELD_NAME];
    const Json::Value &field_value = json_value[FIELD_FIELD_NAME];
    if (!name_value.isString()) {
        debugPrint("IndexMeta:index name is not a string. json value=%s\n", name_value.toStyledString().c_str());
        return Re::GenericError;
    }
    if (!field_value.isString()) {
        debugPrint("IndexMeta:field name of index [%s] is not a string. json value=%s\n",
                   name_value.asCString(),
                   field_value.toStyledString().c_str());
        return Re::GenericError;
    }
    const FieldMeta *field = table_meta.getField(field_value.asCString());
    if (field == nullptr) {
        debugPrint("IndexMeta:deserialize index [%s]: no such field: %s\n", name_value.asCString(),
                   field_value.asCString());
        return Re::SchemaFieldMissing;
    }
    return index_meta.initialize(name_value.asCString(), *field);
}