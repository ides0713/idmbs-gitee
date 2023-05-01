#pragma once
#include <stdint.h>                                   // for int32_t
#include <cstring>                                    // for size_t
#include <filesystem>                                 // for path
#include <string>                                     // for string
#include <iosfwd>                                     // for istream, ostream
#include <vector>                                     // for vector

#include "field.h"                                    // for FieldMeta
#include "index.h"                                    // for IndexMeta
#include "../common/re.h"  // for Re

class DiskBufferPool;
class RecordFileHandler;
class RecordFileScanner;
class Txn;
class CLogManager;
struct AttrInfo;
struct Value;

#define TABLE_NAME_MAX_LEN 20
class TableMeta
{
public:
    TableMeta() : record_size_(0) {}
    TableMeta(const TableMeta &other);
    Re Init(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos);
    int Serialize(std::ostream &ostream) const;
    int Deserialize(std::istream &istream);
    [[nodiscard]] int GetSerialSize() const;
    [[nodiscard]] const FieldMeta *GetTxnField() const;
    [[nodiscard]] const FieldMeta *GetField(int index) const;
    const FieldMeta *GetField(const char *field_name) const;
    [[nodiscard]] const FieldMeta *GetField(std::string field_name) const;
    [[nodiscard]] const char *GetTableName() const { return table_name_.c_str(); }
    [[nodiscard]] int GetFieldsNum() const { return fields_.size(); }
    [[nodiscard]] int GetRecordSize() const { return record_size_; }
    [[nodiscard]] const std::vector<FieldMeta> *GetFields() const { return &fields_; }
    [[nodiscard]] const IndexMeta* GetIndex(int i);
    [[nodiscard]] const IndexMeta* GetIndex(const char * index_name);
    [[nodiscard]] const IndexMeta* GetIndexByField(const char * field_name);
public:
    static int GetSysFieldsNum();

private:
    std::string table_name_;
    std::vector<FieldMeta> fields_;// 包含sys_fields
    std::vector<IndexMeta> indexes_;
    int record_size_;

private:
private:
    static std::vector<FieldMeta> sys_fields;

private:
    static Re InitializeSysFields();
};
class Table
{
public:
    Table() : data_buffer_pool_(nullptr), record_handler_(nullptr), clog_manager_(nullptr) {}
    /// @brief create an not existed table within params
    Re Init(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
            const AttrInfo *attr_infos, CLogManager *clog_manager);
    /// @brief open an exist table from existed table meta file(open table)
    Re Init(std::filesystem::path database_path, const char *table_name, CLogManager *clog_manager);
    /// @brief get table name from table(table meta)
    const char *GetTableName() const { return table_meta_.GetTableName(); }
    const TableMeta &GetTableMeta() const { return table_meta_; }
    Re InsertRecord(Txn *txn, int values_num, const Value *values);
    Re DeleteRecord(Txn *txn, class Record *record);
    Re CreateIndex(Txn* txn,const char * index_name,const char * attr_name);
    RecordFileHandler *GetRecordFileHandler() { return record_handler_; }
    Re GetRecordFileScanner(RecordFileScanner &scanner);
    void Destroy();

private:
    std::filesystem::path database_path_;
    TableMeta table_meta_;
    DiskBufferPool *data_buffer_pool_; /// 数据文件关联的buffer pool
    RecordFileHandler *record_handler_;/// 记录操作
    std::vector<Index *> indexes_;
    CLogManager *clog_manager_;

private:
    Re InitRecordHandler(const char *base_dir);
    Re InitRecordHandler(std::filesystem::path base_dir);
    Re MakeRecord(int values_num, const Value *values, char *&record_data);
    Re InsertRecord(Txn *txn, class Record *rec);
};
