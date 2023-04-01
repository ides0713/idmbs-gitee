#include "table.h"
#include <utility>
#include "txn.h"
#include <cstdio>
#include <iostream>
#include <fstream>

static const Json::StaticString FIELD_TABLE_NAME("table_name");
static const Json::StaticString FIELD_FIELDS("fields");
static const Json::StaticString FIELD_INDEXES("indexes");

std::vector<FieldMeta> TableMeta::sys_fields_;

TableMeta::TableMeta(const TableMeta &other)
        : table_name_(other.table_name_), fields_(other.fields_), indexes_(other.indexes_),
          record_size_(other.record_size_) {}

Re TableMeta::initialize(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos) {
    if (strlen(table_name) == 0 or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        debugPrint("TableMeta:initialize table_meta:%s failed,table name:%s is not valid\n", table_name, table_name);
        return Re::Fail;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        debugPrint("TableMeta:initialize table_meta:%s failed,attr is not valid\n", table_name);
        return Re::Fail;
    }
    if (sys_fields_.empty()) {
        Re r = initializeSysFields();
        if (r != Re::Success) {
            debugPrint("TableMeta:initialize table_meta:%s failed,initialize system fields failed\n", table_name);
            return Re::Fail;
        }
    }
    fields_.resize(attr_infos_num + sys_fields_.size());
    for (size_t i = 0; i < sys_fields_.size(); i++)
        fields_[i] = sys_fields_[i];
    int field_offset = sys_fields_.back().getOffset() + sys_fields_.back().getLen();
    for (int i = 0; i < attr_infos_num; i++) {
        const AttrInfo &attr_info = attr_infos[i];
        Re r = fields_[i + sys_fields_.size()].initialize(attr_info.attr_name, attr_info.attr_type, field_offset,
                                                          attr_info.attr_len, true);
        if (r != Re::Success) {
            debugPrint("TableMeta:Failed to init field meta.table name=%s,field name:%s\n", table_name,
                       attr_info.attr_name);
            return Re::Fail;
        }
        field_offset += attr_info.attr_len;
    }
    record_size_ = field_offset;
    table_name_ = table_name;
    debugPrint("TableMeta:initialized table meta.table name=%s\n", table_name);
    return Re::Success;
}

Re TableMeta::initializeSysFields() {
    sys_fields_.reserve(1);
    FieldMeta field_meta;
    Re r = field_meta.initialize(Txn::getTxnFieldName(), Txn::getTxnFieldType(), 0, Txn::getTxnFieldLen(), false);
    if (r != Re::Success) {
        debugPrint("TableMeta:failed to initialize system fields\n");
        return Re::Fail;
    }
    sys_fields_.push_back(field_meta);
    return Re::Success;
}


Re Table::initialize(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
                     const AttrInfo *attr_infos) {
    if (strlen(table_name) == 0 or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        debugPrint("Table:initialize table:%s failed,table getTableName:%s is not valid \n", table_name, table_name);
        return Re::Fail;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        debugPrint("Table:initialize table:%s failed,attr is not valid\n", table_name);
        return Re::Fail;
    }
    namespace fs = std::filesystem;
    std::string table_file_name = std::string(table_name) + ".table";
    fs::path table_file_path = fs::path(std::move(database_path)).append(table_file_name);
    FILE *f = fopen(table_file_path.c_str(), "r");
    if (f != nullptr) {
        debugPrint("Table:initialize failed,table:%s already exists\n", table_name);
        fclose(f);
        return Re::Fail;
    } else
        fclose(f);
    f = fopen(table_file_path.c_str(), "a");
    fclose(f);
    Re r = table_meta_.initialize(table_name, attr_infos_num, attr_infos);
    if (r != Re::Success) {
        debugPrint("Table:initialize failed,table:%s\n", table_name);
        return Re::Fail;
    }
    std::fstream file_stream;
    file_stream.open(table_file_path, std::ios_base::out | std::ios_base::binary);
    if (!file_stream.is_open()) {
        debugPrint("Table:Failed to open file for write. file name=%s, errmsg=%s\n", table_file_path.c_str(),
                   strerror(errno));
        return Re::Fail;
    }
    return Re::Success;
}
