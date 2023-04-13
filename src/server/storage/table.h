#pragma once

#include <cstring>
#include <string>
#include <filesystem>
#include <jsoncpp/json/json.h>
#include "../parse/parse_defs.h"
#include "field.h"
#include "index.h"

struct RecordId;

class Record;

class DiskBufferPool;

class RecordFileHandler;

class RecordFileScanner;

class ConditionFilter;

class DefaultConditionFilter;

class IndexScanner;

class RecordDeleter;

class Txn;

class CLogManager;

#define TABLE_NAME_MAX_LEN 20

class TableMeta {
public:
    TableMeta() : record_size_(0) {}

    TableMeta(const TableMeta &other);

    Re init(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos);

    int serialize(std::ostream &ostream) const;

    int deserialize(std::istream &istream);

    [[nodiscard]] int getSerialSize() const;

    [[nodiscard]] const FieldMeta *getTxnField() const;

    [[nodiscard]] const FieldMeta *getField(int index) const;

    const FieldMeta *getField(const char *field_name) const;

    [[nodiscard]] const FieldMeta *getField(std::string field_name) const;

    [[nodiscard]] std::string getTableName() const { return table_name_; }

    [[nodiscard]] int getFieldsNum() const { return fields_.size(); }

    [[nodiscard]] int getRecordSize() const { return record_size_; }

public:
    static int getSysFieldsNum();

private:
    std::string table_name_;
    std::vector<FieldMeta> fields_;  // 包含sys_fields
    std::vector<IndexMeta> indexes_;
    int record_size_;
    //todo: why use static variable?
private:
private:
    static std::vector<FieldMeta> sys_fields_;
private:
    static Re initializeSysFields();
};

class Table {
public:
    Table() : data_buffer_pool_(nullptr), record_handler_(nullptr), clog_manager_(nullptr) {}

    /// @brief create a not existed table within params
    Re init(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
            const AttrInfo *attr_infos, CLogManager *clog_manager);

    /// @brief open an exist table from existed table meta file(open table)
    Re init(std::filesystem::path database_path, const char *table_name, CLogManager *clog_manager);

    /// @brief getFrame table name from table(table meta)
    std::string getTableName() { return table_meta_.getTableName(); }

    TableMeta getTableMeta() const { return table_meta_; }

    Re insertRecord(Txn *txn, int values_num, const Value *values);

private:
    std::filesystem::path database_path_;
    TableMeta table_meta_;
    DiskBufferPool *data_buffer_pool_;  /// 数据文件关联的buffer pool
    RecordFileHandler *record_handler_; /// 记录操作
    std::vector<Index *> indexes_;
    CLogManager *clog_manager_;
private:
    Re initRecordHandler(const char *base_dir);

    Re initRecordHandler(std::filesystem::path base_dir);

    Re makeRecord(int values_num, const Value *values, char *&record_data);

    Re insertRecord(Txn *txn, class Record *rec);
};
