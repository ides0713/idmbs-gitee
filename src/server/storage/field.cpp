#include "field.h"
#include <jsoncpp/json/json.h>
#include "../../common/common_defs.h"
#include "table.h"

const static Json::StaticString FIELD_NAME("name");
const static Json::StaticString FIELD_TYPE("getExprType");
const static Json::StaticString FIELD_OFFSET("offset");
const static Json::StaticString FIELD_LEN("len");
const static Json::StaticString FIELD_VISIBLE("visible");

const char *ATTR_TYPE_NAME[] = {"undefined", "chars", "ints", "floats", "dates"};

const char *castAttrTypeString(AttrType type) {
    if (type >= AttrType::Undefined && type <= AttrType::Dates)
        return ATTR_TYPE_NAME[type];
    return "unknown";
}

AttrType castStringAttrType(const char *s) {
    for (unsigned int i = 0; i < sizeof(ATTR_TYPE_NAME) / sizeof(ATTR_TYPE_NAME[0]); i++)
        if (0 == strcmp(ATTR_TYPE_NAME[i], s))
            return (AttrType) i;
    return AttrType::Undefined;
}

Re FieldMeta::init(const char *field_name, AttrType attr_type, int attr_offset, int attr_len, bool visible) {
    if (strlen(field_name) == 0) {
        debugPrint("FieldMeta:field_meta name:%s invalid\n", field_name);
        return Re::InvalidArgument;
    }
    if (attr_type == AttrType::Undefined or attr_offset < 0 or attr_len <= 0) {
        debugPrint("FieldMeta:field_meta invalid args getAttrType=%d attr_offset=%d attr_len=%d\n", attr_type,
                   attr_offset, attr_len);
        return Re::InvalidArgument;
    }
    field_name_ = field_name;
    attr_type_ = attr_type, len_ = attr_len, offset_ = attr_offset;
    visible_ = visible;
    debugPrint("FieldMeta:init a field_meta with name:%s\n", field_name);
    return Re::Success;
}

void FieldMeta::toJson(Json::Value &json_value) const {
    json_value[FIELD_NAME] = field_name_;
    json_value[FIELD_TYPE] = castAttrTypeString(attr_type_);
    json_value[FIELD_OFFSET] = offset_;
    json_value[FIELD_LEN] = len_;
    json_value[FIELD_VISIBLE] = visible_;
}

Re FieldMeta::fromJson(const Json::Value &json_value, FieldMeta &field) {
    if (!json_value.isObject()) {
        debugPrint("FieldMeta:failed to deserialize getFieldName. json is not an object. json value=%s\n",
                   json_value.toStyledString().c_str());
        return Re::GenericError;
    }

    const Json::Value &name_value = json_value[FIELD_NAME];
    const Json::Value &type_value = json_value[FIELD_TYPE];
    const Json::Value &offset_value = json_value[FIELD_OFFSET];
    const Json::Value &len_value = json_value[FIELD_LEN];
    const Json::Value &visible_value = json_value[FIELD_VISIBLE];

    if (!name_value.isString()) {
        debugPrint("FieldMeta:getFieldName name is not a string. json value=%s\n", name_value.toStyledString().c_str());
        return Re::GenericError;
    }
    if (!type_value.isString()) {
        debugPrint("FieldMeta:Field getExprType is not a string. json value=%s\n", type_value.toStyledString().c_str());
        return Re::GenericError;
    }

    if (!offset_value.isInt()) {
        debugPrint("FieldMeta:offset is not an integer. json value=%s\n", offset_value.toStyledString().c_str());
        return Re::GenericError;
    }
    if (!len_value.isInt()) {
        debugPrint("FieldMeta:len is not an integer. json value=%s\n", len_value.toStyledString().c_str());
        return Re::GenericError;
    }
    if (!visible_value.isBool()) {
        debugPrint("FieldMeta:visible getFieldName is not a bool value. json value=%s\n",
                   visible_value.toStyledString().c_str());
        return Re::GenericError;
    }

    AttrType type = castStringAttrType(type_value.asCString());
    if (type == AttrType::Undefined) {
        debugPrint("FieldMeta:getFrame invalid getFieldName getExprType. getExprType=%d\n", type);
        return Re::GenericError;
    }

    const char *field_name = name_value.asCString();
    int attr_offset = offset_value.asInt();
    int attr_len = len_value.asInt();
    bool visible = visible_value.asBool();
    return field.init(field_name, type, attr_offset, attr_len, visible);
}

std::string Field::getTableName()const  {
    return table_->getTableName();
}
