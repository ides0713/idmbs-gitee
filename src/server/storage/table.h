#pragma once

#include <cstring>
#include <string>
#include "ClogManager.h"
#include "buffer_pool.h"
#include <filesystem>
#include "../parse/parse_defs.h"
#include "Field.h"
#include "Index.h"
#include <jsoncpp/json/json.h>

#define TABLE_NAME_MAX_LEN 20

class TableMeta {
public:
    TableMeta() : record_size_(0) {}

    Re initialize(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos);

private:
    std::string table_name_;
    std::vector<FieldMeta> fields_;  // 包含sys_fields
    std::vector<IndexMeta> indexes_;
    int record_size_;
    //todo: why use static variable?
private:
    static std::vector<FieldMeta> sys_fields_;
private:
    static Re initializeSysFields();
};

class Table {
public:
    Table() = default;

    Re initialize(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
                  const AttrInfo *attr_infos);

    std::string getTableName() { return table_name_; }

private:
    std::filesystem::path database_path_;
    std::string table_name_;
    TableMeta table_meta_;
    ClogManager *clog_manager_;
};
