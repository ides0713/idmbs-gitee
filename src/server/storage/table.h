#pragma once
#include "../common/re.h"// for Re
#include "field.h"       // for FieldMeta
#include "index.h"       // for IndexMeta
#include <cstring>       // for size_t
#include <filesystem>    // for path
#include <iosfwd>        // for istream, ostream
#include <stdint.h>      // for int32_t
#include <string>        // for string
#include <vector>        // for vector
class DiskBufferPool;
class RecordFileHandler;
class RecordFileScanner;
class Txn;
class CLogManager;
class ConditionFilter;
class DefaultConditionFilter;
class CompositeConditionFilter;
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
    [[nodiscard]] const FieldMeta *GetFieldByOffset(int offset) const;
    [[nodiscard]] const char *GetTableName() const { return table_name_.c_str(); }
    [[nodiscard]] int GetFieldsNum() const { return fields_.size(); }
    [[nodiscard]] int GetRecordSize() const { return record_size_; }
    [[nodiscard]] const std::vector<FieldMeta> *GetFields() const { return &fields_; }
    [[nodiscard]] const IndexMeta *GetIndex(int i) const;
    [[nodiscard]] const IndexMeta *GetIndex(const char *index_name) const;
    [[nodiscard]] const IndexMeta *GetIndexByField(const char *field_name) const;

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
    Re CreateIndex(Txn *txn, const char *index_name, const char *attr_name);
    RecordFileHandler *GetRecordFileHandler() const { return record_handler_; }
    Re GetRecordFileScanner(RecordFileScanner &scanner);
    void Destroy();
    Re Drop();
    Re Sync();
    Re ScanRecord(Txn *txn, ConditionFilter *filter, int limit, void *context,
                  void (*record_reader)(const char *data, void *context));
    Index *GetIndex(const char *index_name) const;
    Index *GetIndexByField(const char *field_name) const;

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
    Re ScanRecord(Txn *txn, ConditionFilter *filter, int limit, void *context,
                  Re (*record_reader)(class Record *record, void *context));
    Re ScanRecordByIndex(Txn *txn, IndexScanner *scanner, ConditionFilter *filter, int limit, void *context,
                         Re (*record_reader)(class Record *record, void *context));
    IndexScanner *FindIndexForScan(const ConditionFilter *filter);
    IndexScanner *FindIndexForScan(const DefaultConditionFilter &filter);
    Re InsertEntryOfIndexes(const char *record, const RecordId &rid);
    Re DeleteEntryOfIndexes(const char *record, const RecordId &rid, bool not_exists_error);
};
class IndexInserter
{
public:
    explicit IndexInserter(Index *index) : index_(index) {}
    Re InsertIndex(class Record *record);

private:
    Index *index_;
};
static Re InsertIndexRecordReaderAdapter(class Record *record, void *context);
class RecordReaderScanAdapter
{
public:
    explicit RecordReaderScanAdapter(void (*record_reader)(const char *data, void *context), void *context)
        : record_reader_(record_reader), context_(context) {}
    void Consume(class Record *record);

private:
    void (*record_reader_)(const char *, void *);
    void *context_;
};
static Re ScanRecordReaderAdapter(class Record *record, void *context);
class RecordDeleter
{
public:
    RecordDeleter(Table &table, Txn *txn) : table_(table), txn_(txn), deleted_count_(0) {}
    Re DeleteRecord(class Record *record);
    int GetDeletedCount() const { return deleted_count_; }

private:
    Table &table_;
    Txn *txn_;
    int deleted_count_;
};