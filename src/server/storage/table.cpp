#include "table.h"
#include "../../common/common_defs.h"// for DebugPrint
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"// for GlobalMa...
#include "../parse/parse_defs.h"      // for Value
#include "b_plus_tree_index.h"
#include "buffer_pool.h" // for GlobalBu...
#include "clog_manager.h"// for CLogManager
#include "condition_filter.h"
#include "record.h"             // for Record
#include "storage_defs.h"       // for GetTable...
#include "txn.h"                // for Txn
#include <algorithm>            // for sort
#include <bits/chrono.h>        // for filesystem
#include <compare>              // for operator<
#include <cstdio>               // for fclose
#include <errno.h>              // for errno
#include <ext/alloc_traits.h>   // for __alloc_...
#include <fstream>              // for fstream
#include <jsoncpp/json/config.h>// for String
#include <jsoncpp/json/reader.h>// for parseFro...
#include <jsoncpp/json/value.h> // for Value
#include <jsoncpp/json/writer.h>// for StreamWr...
#include <memory>               // for allocato...
#include <utility>              // for move
#include <filesystem>
static const Json::StaticString FIELD_TABLE_NAME("table_name");
static const Json::StaticString FIELD_FIELDS("fields");
static const Json::StaticString FIELD_INDEXES("indexes");
std::vector<FieldMeta> TableMeta::sys_fields;
TableMeta::TableMeta(const TableMeta &other)
    : table_name_(other.table_name_), fields_(other.fields_), indexes_(other.indexes_),
      record_size_(other.record_size_) {
}
Re TableMeta::Init(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos) {
    if (StrBlank(table_name) or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        DebugPrint("TableMeta:init table_meta:%s failed,table name:%s is not valid\n", table_name, table_name);
        return Re::InvalidArgument;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        DebugPrint("TableMeta:init table_meta:%s failed,attr is not valid\n", table_name);
        return Re::InvalidArgument;
    }
    if (sys_fields.empty()) {
        Re r = InitializeSysFields();
        if (r != Re::Success) {
            DebugPrint("TableMeta:init table_meta:%s failed,init system fields failed\n", table_name);
            return r;
        }
    }
    int sys_fields_size = sys_fields.size();
    fields_.resize(attr_infos_num + sys_fields_size);
    for (size_t i = 0; i < sys_fields_size; i++)
        fields_[i] = sys_fields[i];
    int field_offset = sys_fields.back().GetOffset() + sys_fields.back().GetLen();
    for (int i = 0; i < attr_infos_num; i++) {
        const AttrInfo &attr_info = attr_infos[i];
        Re r = fields_[i + sys_fields_size].Init(attr_info.attr_name, attr_info.attr_type, field_offset,
                                                 attr_info.attr_len, true);
        if (r != Re::Success) {
            DebugPrint("TableMeta:Failed to init field meta.table name=%s,getFieldName name:%s\n", table_name,
                       attr_info.attr_name);
            return r;
        }
        field_offset += attr_info.attr_len;
    }
    record_size_ = field_offset, table_name_ = table_name;
    table_name_ = std::string(table_name);
    DebugPrint("TableMeta:initialized table meta.table name=%s\n", table_name);
    return Re::Success;
}
Re TableMeta::InitializeSysFields() {
    sys_fields.reserve(1);
    FieldMeta field_meta;
    Re r = field_meta.Init(Txn::GetTxnFieldName(), Txn::GetTxnFieldType(), 0, Txn::GetTxnFieldLen(), false);
    if (r != Re::Success) {
        DebugPrint("TableMeta:failed to init system fields\n");
        return r;
    }
    sys_fields.push_back(field_meta);
    return Re::Success;
}
int TableMeta::Serialize(std::ostream &ostream) const {
    Json::Value table_value;
    table_value[FIELD_TABLE_NAME] = table_name_;
    Json::Value fields_value;
    for (const FieldMeta &field: fields_) {
        Json::Value field_value;
        field.ToJson(field_value);
        fields_value.append(std::move(field_value));
    }
    table_value[FIELD_FIELDS] = std::move(fields_value);
    Json::Value indexes_value;
    for (const auto &index: indexes_) {
        Json::Value index_value;
        index.ToJson(index_value);
        indexes_value.append(std::move(index_value));
    }
    table_value[FIELD_INDEXES] = std::move(indexes_value);
    Json::StreamWriterBuilder builder;
    Json::StreamWriter *writer = builder.newStreamWriter();
    std::streampos old_pos = ostream.tellp();
    writer->write(table_value, &ostream);
    int ret = (int) (ostream.tellp() - old_pos);
    delete writer;
    return ret;
}
int TableMeta::Deserialize(std::istream &istream) {
    if (sys_fields.empty()) {
        InitializeSysFields();
    }
    Json::Value table_value;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::streampos old_pos = istream.tellg();
    if (!Json::parseFromStream(builder, istream, &table_value, &errors)) {
        DebugPrint("TableMeta:failed to deserialize table meta. error=%s\n", errors.c_str());
        return -1;
    }
    const Json::Value &table_name_value = table_value[FIELD_TABLE_NAME];
    if (!table_name_value.isString()) {
        DebugPrint("TableMeta:invalid table name. json value=%s\n", table_name_value.toStyledString().c_str());
        return -1;
    }
    std::string table_name = table_name_value.asString();
    const Json::Value &fields_value = table_value[FIELD_FIELDS];
    if (!fields_value.isArray() or fields_value.size() <= 0) {
        DebugPrint("TableMeta:invalid table meta. fields is not array, json value=%s\n",
                   fields_value.toStyledString().c_str());
        return -1;
    }
    int field_num = fields_value.size();
    std::vector<FieldMeta> fields(field_num);
    for (int i = 0; i < field_num; i++) {
        FieldMeta &field = fields[i];
        const Json::Value &field_value = fields_value[i];
        Re r = FieldMeta::FromJson(field_value, field);
        if (r != Re::Success) {
            DebugPrint("TableMeta:failed to deserialize table meta. table name =%s\n", table_name.c_str());
            return -1;
        }
    }
    std::sort(fields.begin(), fields.end(),
              [](const FieldMeta &f_1, const FieldMeta &f_2) { return f_1.GetOffset() < f_2.GetOffset(); });
    table_name_.swap(table_name);
    fields_.swap(fields);
    record_size_ = fields_.back().GetOffset() + fields_.back().GetLen() - fields_.begin()->GetOffset();
    const Json::Value &indexes_value = table_value[FIELD_INDEXES];
    if (!indexes_value.empty()) {
        if (!indexes_value.isArray()) {
            DebugPrint("TableMeta:invalid table meta. indexes is not array, json value=%s\n",
                       fields_value.toStyledString().c_str());
            return -1;
        }
        const int index_num = indexes_value.size();
        std::vector<IndexMeta> indexes(index_num);
        for (int i = 0; i < index_num; i++) {
            IndexMeta &index = indexes[i];
            const Json::Value &index_value = indexes_value[i];
            Re r = IndexMeta::FromJson(*this, index_value, index);
            if (r != Re::Success) {
                DebugPrint("TableMeta:failed to deserialize table meta. table name=%s\n", table_name.c_str());
                return -1;
            }
        }
        indexes_.swap(indexes);
    }
    return (int) (istream.tellg() - old_pos);
}
int TableMeta::GetSerialSize() const {
    return -1;
}
const FieldMeta *TableMeta::GetField(int index) const {
    return &fields_[index];
}
const FieldMeta *TableMeta::GetField(const char *field_name) const {
    if (strlen(field_name) == 0) {
        DebugPrint("TableMeta:getFrame field:%s failed,field_name invalid\n", field_name);
        return nullptr;
    }
    for (const FieldMeta &field: fields_)
        if (strcmp(field.GetFieldName(), field_name) == 0)
            return &field;
    return nullptr;
}
const FieldMeta *TableMeta::GetField(std::string field_name) const {
    return GetField(field_name.c_str());
}
const FieldMeta *TableMeta::GetFieldByOffset(int offset) const {
    for (const FieldMeta &field: fields_)
        if (field.GetOffset() == offset)
            return &field;
    return nullptr;
}
const IndexMeta *TableMeta::GetIndex(int i) const {
    if (i < indexes_.size() and i >= 0)
        return &indexes_[i];
    return nullptr;
}
const IndexMeta *TableMeta::GetIndex(const char *index_name) const {
    for (int i = 0; i < indexes_.size(); i++)
        if (strcmp(index_name, indexes_[i].GetIndexName()) == 0)
            return &indexes_[i];
    return nullptr;
}
const IndexMeta *TableMeta::GetIndexByField(const char *field_name) const {
    for (int i = 0; i < indexes_.size(); i++)
        if (strcmp(field_name, indexes_[i].GetFieldName()) == 0)
            return &indexes_[i];
    return nullptr;
}
int TableMeta::GetSysFieldsNum() {
    return sys_fields.size();
}
const FieldMeta *TableMeta::GetTxnField() const {
    return &fields_[0];
}
Re Table::Init(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
               const AttrInfo *attr_infos, CLogManager *clog_manager) {
    if (StrBlank(table_name) or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        DebugPrint("Table:init table:%s failed,table getTableName:%s is not valid \n", table_name, table_name);
        return Re::InvalidArgument;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        DebugPrint("Table:init table:%s failed,attr is not valid\n", table_name);
        return Re::InvalidArgument;
    }
    namespace fs = std::filesystem;
    fs::path table_meta_file_path = GetTableMetaFilePath(database_path, table_name);
    FILE *f = fopen(table_meta_file_path.c_str(), "r");
    if (f != nullptr) {
        printf("table already exists\n");
        DebugPrint("Table:init failed,table:%s already exists\n", table_name);
        fclose(f);
        return Re::SchemaTableExist;
    }
    f = fopen(table_meta_file_path.c_str(), "a");
    fclose(f);
    Re r = table_meta_.Init(table_name, attr_infos_num, attr_infos);
    if (r != Re::Success) {
        DebugPrint("Table:init failed,table:%s\n", table_name);
        return r;
    }
    std::fstream file_stream;
    file_stream.open(table_meta_file_path, std::ios_base::out | std::ios_base::binary);
    if (!file_stream.is_open()) {
        DebugPrint("Table:Failed to open file for write. file name=%s, errmsg=%s\n", table_meta_file_path.c_str(),
                   strerror(errno));
        return Re::IoErr;
    }
    table_meta_.Serialize(file_stream);
    file_stream.close();
    std::string data_file_name = std::string(table_name) + ".data";
    fs::path data_file_path = fs::path(database_path).append(data_file_name);
    GlobalBufferPoolManager &bpm = GlobalManagers::GetGlobalBufferPoolManager();
    r = bpm.CreateFile(data_file_path.c_str());
    if (r != Re::Success) {
        DebugPrint("Table:failed to create disk buffer pool of data file. file name=%s\n", data_file_name.c_str());
        return r;
    }
    r = InitRecordHandler(database_path);
    if (r != Re::Success) {
        DebugPrint("Table:failed to create table %s due to init record handler failed.", data_file_name.c_str());
        // don't need to remove the data_file
        return r;
    }
    database_path_ = database_path;
    clog_manager_ = clog_manager;
    DebugPrint("Table:successfully create table %s:%s\n", database_path_.c_str(), table_meta_.GetTableName());
    return Re::Success;
}
Re Table::InitRecordHandler(const char *base_dir) {
    namespace fs = std::filesystem;
    std::string data_file_name = std::string(table_meta_.GetTableName()) + ".data";
    fs::path data_file_path = fs::path(base_dir).append(data_file_name);
    GlobalBufferPoolManager &bpm = GlobalManagers::GetGlobalBufferPoolManager();
    Re r = bpm.OpenFile(data_file_path, data_buffer_pool_);
    if (r != Re::Success) {
        DebugPrint("TableMeta:failed to open disk buffer pool for file:%s. r=%d\n", data_file_name.c_str(), r);
        return r;
    }
    record_handler_ = new RecordFileHandler();
    r = record_handler_->Init(data_buffer_pool_);
    if (r != Re::Success) {
        DebugPrint("TableMeta:failed to init record handler. r=%d:%s\n", r, StrRe(r));
        bpm.CloseFile(data_file_path);
        data_buffer_pool_ = nullptr;
        delete record_handler_;
        record_handler_ = nullptr;
        return r;
    }
    return Re::Success;
}
Re Table::InitRecordHandler(std::filesystem::path base_dir) {
    return InitRecordHandler(base_dir.c_str());
}
Re Table::Init(std::filesystem::path database_path, const char *table_name, CLogManager *clog_manager) {
    namespace fs = std::filesystem;
    std::fstream f_stream;
    fs::path table_meta_file_path = GetTableMetaFilePath(database_path, table_name);
    f_stream.open(table_meta_file_path.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!f_stream.is_open()) {
        DebugPrint("Table:failed to open meta file for read. file name=%s, errmsg=%s\n", table_meta_file_path.c_str(),
                   strerror(errno));
        return Re::IoErr;
    }
    if (table_meta_.Deserialize(f_stream) < 0) {
        DebugPrint("Table:failed to deserialize table meta. file name=%s\n", table_meta_file_path.c_str());
        f_stream.close();
        return Re::GenericError;
    }
    f_stream.close();
    Re r = InitRecordHandler(database_path);
    if (r != Re::Success) {
        DebugPrint("Table:failed to open table %s due to init record handler failed.\n", table_meta_.GetTableName());
        // don't need to remove the data_file
        return r;
    }
    database_path_ = database_path;
    if (clog_manager_ == nullptr)
        clog_manager_ = clog_manager;
    return Re::Success;
}
Re Table::InsertRecord(Txn *txn, int values_num, const Value *values) {
    char *record_data;
    MakeRecord(values_num, values, record_data);
    class Record *rec = new class Record;
    rec->SetData(record_data);
    Re r = InsertRecord(txn, rec);
    delete[] record_data;
    return r;
}
Re Table::DeleteRecord(Txn *txn, class Record *record) {
    Re r = DeleteEntryOfIndexes(record->GetData(), record->GetRecordId(), false);// again refer to commit_delete
    if (r != Re::Success) {
        DebugPrint("Table:failed to delete indexes of record (rid=%d.%d). r=%d:%s\n", record->GetRecordId().page_id,
                   record->GetRecordId().slot_id, r, StrRe(r));
        return r;
    }
    r = record_handler_->DeleteRecord(&record->GetRecordId());
    if (r != Re::Success) {
        DebugPrint("Table:failed to delete record rid=%d,%d r=%d,%s\n", record->GetRecordId().page_id,
                   record->GetRecordId().slot_id, r, StrRe(r));
        return r;
    }
    if (txn != nullptr) {
        txn->DeleteRecord(this, record);
        CLogRecord *clog_record = nullptr;
        r = clog_manager_->MakeRecord(CLogType::RedoDelete, txn->GetTxnId(), clog_record, GetTableName(), 0, record);
        if (r != Re::Success) {
            DebugPrint("Table:failed to create a clog record r=%d,%s\n", r, StrRe(r));
            return r;
        }
        r = clog_manager_->AppendRecord(clog_record);
        if (r != Re::Success) {
            DebugPrint("Table:failed to append clog record r=%d,%s\n", r, StrRe(r));
            return r;
        }
    }
    return Re::Success;
}
Re Table::CreateIndex(Txn *txn, const char *index_name, const char *attr_name) {
    namespace fs = std::filesystem;
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (StrBlank(index_name) or StrBlank(attr_name)) {
        DebugPrint("Table:create index failed,invalid args\n");
        return Re::InvalidArgument;
    }
    if (table_meta_.GetIndex(index_name) != nullptr or table_meta_.GetIndexByField(attr_name) != nullptr) {
        DebugPrint("Table:create index failed,index already exists with same name or on target attr\n");
        return Re::SchemaIndexExist;
    }
    const FieldMeta *field_meta = table_meta_.GetField(attr_name);
    if (field_meta == nullptr) {
        DebugPrint("Table:create index failed,no such field '%s.%s'\n", GetTableName(), attr_name);
        gmm.SetResponse("CREATE INDEX FAILED,NO SUCH FIELD '%s.%s'.\n", GetTableName(), attr_name);
        return Re::SchemaFieldMissing;
    }
    IndexMeta new_index_meta;
    Re r = new_index_meta.Init(index_name, *field_meta);
    if (r != Re::Success) {
        DebugPrint("Table:create index failed,init index meta on index failed\n");
        return r;
    }
    // 创建索引相关数据
    BplusTreeIndex *index = new BplusTreeIndex();
    fs::path index_file_path = GetTableIndexFilePath(database_path_.c_str(), GetTableName(), index_name);
    r = index->Create(index_file_path.c_str(), new_index_meta, *field_meta);
    if (r != Re::Success) {
        delete index;
        DebugPrint("Table:failed to create bplus tree index. file name=%s, r=%d:%s\n", index_file_path.c_str(), r,
                   StrRe(r));
        return r;
    }
    // 遍历当前的所有数据,并将数据插入这个索引
    IndexInserter index_inserter(index);
    r = ScanRecord(txn, nullptr, -1, &index_inserter, InsertIndexRecordReaderAdapter);
    if (r != Re::Success) {
        // rollback
        Re r_2 = index->Close();
        if (r_2 != Re::Success) {
            DebugPrint("Table:failed to insert index to all records and roll back failed,r=%d:%s\n", r, StrRe(r));
            gmm.SetResponse("CREATE INDEX FAILED,ROLL BACK FAILED.\n");
            return r;
        }
        index = nullptr;
        DebugPrint("Table:failed to insert index to all records. table=%s, r=%d:%s\n", GetTableName(), r, StrRe(r));
        return r;
    }
    indexes_.push_back(index);
    return Re::Success;
}
Re Table::GetRecordFileScanner(RecordFileScanner &scanner) {
    Re r = scanner.Init(*data_buffer_pool_, nullptr);
    if (r != Re::Success)
        DebugPrint("Table:failed to open scanner. r=%d:%s", r, StrRe(r));
    return Re::Success;
}
void Table::Destroy() {
    if (record_handler_ != nullptr) {
        record_handler_->Destroy();
        record_handler_ = nullptr;
    }
    if (data_buffer_pool_ != nullptr) {
        // GlobalBufferPoolManager& bpm = GlobalManagers::globalBufferPoolManager();
        // std::filesystem::path p = getTableDataFilePath(database_path_, table_meta_.getTableName().c_str());
        // printf("p.cstr() is %s\n",p.c_str());
        // data_buffer_pool_->checkAllPages();
        // bpm.closeFile(std::string(p.c_str()));
        // data_buffer_pool_->checkAllPages();
        data_buffer_pool_ = nullptr;
    }
    DebugPrint("Table:table has been destroyed\n");
}
Re Table::Drop() {
    //TODO: drop table todo here\n
    assert(false);
    return Re::GenericError;
    // namespace fs=std::filesystem;
    // Re r = Sync();//刷新所有脏页
    // if (r != Re::Success)
    //     return r;
    // GlobalBufferPoolManager& bpm=GlobalManagers::GetGlobalBufferPoolManager();
    // std::string meta_file_path = GetTableMetaFilePath(database_path_, GetTableName());
    // r=bpm.CloseFile(meta_file_path.c_str());
    // if(r!=Re::Success){
    //     DebugPrint("Table:drop failed,failed to close file %s from buffer pool\n",meta_file_path.c_str());
        
    //     return r;
    // }
    // if (!fs::remove(meta_file_path)) {
    //     DebugPrint("Table:drop failed,failed to remove meta file=%s\n",meta_file_path.c_str());
        
    //     return Re::GenericError;
    // }
    // std::string data_file_path= GetTableDataFilePath(database_path_,GetTableName());
    // bpm.CloseFile(data_file_path.c_str());
    // if (!fs::remove(data_file_path)) {
    //     DebugPrint("Table:drop failed,failed to remove data file=%s, errno=%d", data_file.c_str(), errno);
    //     return Re::GenericError;
    // }
    // //TODO close file from disk buffer pool?
    // BufferPoolManager &bpm = BufferPoolManager::instance();
    // r = bpm.close_file(path.c_str());
    // const int index_num = table_meta_.index_num();
    // for (int i = 0; i < index_num; i++) {
    //     ((BplusTreeIndex *) indexes_[i])->close();
    //     const IndexMeta *index_meta = table_meta_.index(i);
    //     std::string index_file = table_index_file(dir, name(), index_meta->name());
    //     if (unlink(index_file.c_str()) != 0) {
    //         DebugPrint("Table:failed to remove index file=%s, errno=%d", index_file.c_str(), errno);
    //         return Re::GenericError;
    //     }
    // }
    // return Re::Success;
}
Re Table::Sync() {
    Re r = Re::Success;
    for (Index *index: indexes_) {
        r = index->Sync();
        if (r != Re::Success) {
            DebugPrint("Table:failed to flush index's pages.table=%s,index=%s,r=%d:%s\n", GetTableName(),
                       index->GetIndexMeta().GetIndexName(), r, StrRe(r));
            return r;
        }
    }
    DebugPrint("Table:sync table over. table=%s\n", GetTableName());
    return r;
}
Re Table::ScanRecord(Txn *txn, ConditionFilter *filter, int limit, void *context,
                     void (*record_reader)(const char *data, void *context)) {
    RecordReaderScanAdapter adapter(record_reader, context);
    return ScanRecord(txn, filter, limit, (void *) &adapter, ScanRecordReaderAdapter);
}
Index *Table::GetIndex(const char *index_name) const {
    for (Index *index: indexes_)
        if (strcmp(index->GetIndexMeta().GetIndexName(), index_name) == 0)
            return index;
    return nullptr;
}
Index *Table::GetIndexByField(const char *field_name) const {
    const TableMeta &table_meta = GetTableMeta();
    const IndexMeta *index_meta = table_meta.GetIndexByField(field_name);
    if (index_meta != nullptr)
        return GetIndex(index_meta->GetIndexName());
    return nullptr;
}
Re Table::InsertRecord(Txn *txn, class Record *rec) {
    if (txn != nullptr)
        txn->Init(this, *rec);
    Re r = record_handler_->InsertRecord(rec->GetData(), table_meta_.GetRecordSize(), &rec->GetRecordId());
    if (r != Re::Success) {
        DebugPrint("Table:insert record failed. table name=%s, r=%d:%s\n", table_meta_.GetTableName(), r, StrRe(r));
        return r;
    }
    if (txn != nullptr) {
        r = txn->InsertRecord(this, rec);
        if (r != Re::Success) {
            DebugPrint("Table:failed to log operation(insertion) to txn\n");
            Re r_2 = record_handler_->DeleteRecord(&rec->GetRecordId());
            if (r_2 != Re::Success)
                DebugPrint("Table:failed to rollback record data"
                           "when insert index entry failed table name=%s, r=%d:%s\n",
                           GetTableName(), r_2, StrRe(r_2));
            return r;
        }
    }
    // todo after implement index of table,we must update the index of the table in order to keep it usable
    r = InsertEntryOfIndexes(rec->GetData(), rec->GetRecordId());
    if (r != Re::Success) {
        //try to roll back
        Re r_2 = DeleteEntryOfIndexes(rec->GetData(), rec->GetRecordId(), true);
        if (r_2 != Re::Success)
            DebugPrint("Table:failed to rollback index data when insert index entries failed. table name=%s, r=%d:%s\n",
                       GetTableName(), r_2, StrRe(r_2));
        r_2 = record_handler_->DeleteRecord(&rec->GetRecordId());
        if (r_2 != Re::Success)
            DebugPrint(
                    "Table:failed to rollback record data when insert index entries failed. table name=%s, r=%d:%s\n",
                    GetTableName(), r_2, StrRe(r_2));
        return r;
    }
    if (txn != nullptr) {
        // append clog record
        CLogRecord *clog_record = nullptr;
        r = clog_manager_->MakeRecord(CLogType::RedoInsert, txn->GetTxnId(), clog_record, GetTableName(),
                                      table_meta_.GetRecordSize(), rec);
        if (r != Re::Success) {
            DebugPrint("Table:failed to create a clog record. r=%d:%s\n", r, StrRe(r));
            return r;
        }
        r = clog_manager_->AppendRecord(clog_record);
        if (r != Re::Success) {
            DebugPrint("Table:failed to append record to clog. r=%d:%s\n", r, StrRe(r));
            return r;
        }
    }
    return Re::Success;
}
Re Table::ScanRecord(Txn *txn, ConditionFilter *filter, int limit, void *context,
                     Re (*record_reader)(class Record *record, void *context)) {
    if (record_reader == nullptr)
        return Re::InvalidArgument;
    if (limit == 0)
        return Re::Success;
    if (limit < 0)
        limit = INT_MAX;
    IndexScanner *index_scanner = FindIndexForScan(filter);
    if (index_scanner != nullptr)
        return ScanRecordByIndex(txn, index_scanner, filter, limit, context, record_reader);
    Re r;
    RecordFileScanner scanner;
    r = scanner.Init(*data_buffer_pool_, filter);
    if (r != Re::Success) {
        DebugPrint("Table:failed to open scanner. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    int record_count = 0;
    class Record record;
    while (scanner.HasNext()) {
        r = scanner.Next(record);
        if (r != Re::Success) {
            DebugPrint("Table:failed to fetch next record. r=%d:%s\n", r, StrRe(r));
            return r;
        }
        if (txn == nullptr or txn->IsVisible(this, &record)) {
            r = record_reader(&record, context);
            if (r != Re::Success)
                break;
            record_count++;
        }
    }
    scanner.Destroy();
    return r;
}
Re Table::ScanRecordByIndex(Txn *txn, IndexScanner *scanner, ConditionFilter *filter, int limit, void *context,
                            Re (*record_reader)(class Record *record, void *context)) {
    Re r = Re::Success;
    RecordId rid;
    class Record record;
    int record_count = 0;
    while (record_count < limit) {
        r = scanner->NextEntry(&rid);
        if (r != Re::Success) {
            if (Re::RecordEof == r) {
                r = Re::Success;
                break;
            }
            DebugPrint("Table:failed to scan table by index. r=%d:%s\n", r, StrRe(r));
            break;
        }
        r = record_handler_->GetRecord(&rid, &record);
        if (r != Re::Success) {
            DebugPrint("Table:failed to fetch record of rid=%d:%d, r=%d:%s\n", rid.page_id, rid.slot_id, r, StrRe(r));
            break;
        }
        if ((txn == nullptr or txn->IsVisible(this, &record)) and (filter == nullptr or filter->Filter(record))) {
            r = record_reader(&record, context);
            if (r != Re::Success) {
                DebugPrint("Table:record reader break the table scanning. r=%d:%s\n", r, StrRe(r));
                break;
            }
        }
        record_count++;
    }
    scanner->Destroy();
    return r;
}
IndexScanner *Table::FindIndexForScan(const ConditionFilter *filter) {
    if (filter == nullptr)
        return nullptr;
    // remove dynamic_cast
    const DefaultConditionFilter *default_condition_filter = dynamic_cast<const DefaultConditionFilter *>(filter);
    if (default_condition_filter != nullptr)
        return FindIndexForScan(*default_condition_filter);
    const CompositeConditionFilter *composite_condition_filter = dynamic_cast<const CompositeConditionFilter *>(filter);
    if (composite_condition_filter != nullptr) {
        int filter_num = composite_condition_filter->GetFilterNum();
        for (int i = 0; i < filter_num; i++) {
            IndexScanner *scanner = FindIndexForScan(&composite_condition_filter->GetFilter(i));
            if (scanner != nullptr)
                return scanner;// 可以找到一个最优的，比如比较符号是=
        }
    }
    return nullptr;
}
IndexScanner *Table::FindIndexForScan(const DefaultConditionFilter &filter) {
    const ConDesc *field_con_desc = nullptr, *value_con_desc = nullptr;
    if (filter.GetLeftConDesc().is_attr and !filter.GetRightConDesc().is_attr)
        field_con_desc = &filter.GetLeftConDesc(), value_con_desc = &filter.GetRightConDesc();
    else if (filter.GetRightConDesc().is_attr and !filter.GetLeftConDesc().is_attr)
        field_con_desc = &filter.GetRightConDesc(), value_con_desc = &filter.GetLeftConDesc();
    if (field_con_desc == nullptr or value_con_desc == nullptr)
        return nullptr;
    const FieldMeta *field_meta = table_meta_.GetFieldByOffset(field_con_desc->attr_offset);
    if (field_meta == nullptr) {
        DebugPrint("Table:can not find field by offset %d. table=%s\n", field_con_desc->attr_offset, GetTableName());
        return nullptr;
    }
    const IndexMeta *index_meta = table_meta_.GetIndexByField(field_meta->GetFieldName());
    if (index_meta == nullptr)
        return nullptr;
    Index *index = GetIndex(index_meta->GetIndexName());
    if (nullptr == index) {
        return nullptr;
    }
    const char *left_key = nullptr, *right_key = nullptr;
    int left_len = 4, right_len = 4;
    bool left_inclusive = false, right_inclusive = false;
    switch (filter.GetCompOp()) {
        case CompOp::EqualTo: {
            left_key = reinterpret_cast<const char *>(value_con_desc->value);
            right_key = reinterpret_cast<const char *>(value_con_desc->value);
            left_inclusive = true, right_inclusive = true;
        } break;
        case CompOp::LessEqual: {
            right_key = reinterpret_cast<const char *>(value_con_desc->value);
            right_inclusive = true;
        } break;
        case CompOp::GreatEqual: {
            left_key = reinterpret_cast<const char *>(value_con_desc->value);
            left_inclusive = true;
        } break;
        case CompOp::LessThan: {
            right_key = reinterpret_cast<const char *>(value_con_desc->value);
            right_inclusive = false;
        } break;
        case CompOp::GreatThan: {
            left_key = reinterpret_cast<const char *>(value_con_desc->value);
            left_inclusive = false;
        } break;
        default: {
            return nullptr;
        }
    }
    if (filter.GetAttrType() == AttrType::Chars) {
        left_len = (left_key != nullptr ? strlen(left_key) : 0);
        right_len = (right_key != nullptr ? strlen(right_key) : 0);
    }
    return index->CreateScanner(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
}
Re Table::InsertEntryOfIndexes(const char *record, const RecordId &rid) {
    Re r = Re::Success;
    for (Index *index: indexes_) {
        r = index->InsertEntry(record, &rid);
        if (r != Re::Success)
            return r;
    }
    return r;
}
Re Table::DeleteEntryOfIndexes(const char *record, const RecordId &rid, bool not_exists_error) {
    Re r = Re::Success;
    for (Index *index: indexes_) {
        r = index->DeleteEntry(record, &rid);
        if (r != Re::Success and (r != Re::RecordInvalidKey or !not_exists_error))
            return r;
    }
    return r;
}
Re Table::MakeRecord(int values_num, const Value *values, char *&record_data) {
    int table_sys_fields_num = table_meta_.GetSysFieldsNum();
    if (values_num != table_meta_.GetFieldsNum() - table_sys_fields_num) {
        DebugPrint("Table:make record failed,values_num not match,values num=%d,table's common fields num=%d\n",
                   values_num, table_meta_.GetFieldsNum() - table_sys_fields_num);
        return Re::SchemaFieldMissing;
    }
    // type check before copy the
    for (int i = 0; i < values_num; i++) {
        const Value &th_value = values[i];
        const FieldMeta *th_field_meta = table_meta_.GetField(i + table_sys_fields_num);
        if (th_value.type != th_field_meta->GetAttrType()) {
            DebugPrint("Table:make record failed,values getExprType %d mismatch fields getExprType %d\n", th_value.type,
                       th_field_meta->GetAttrType());
            return Re::SchemaFieldTypeMismatch;
        }
    }
    record_data = new char[table_meta_.GetRecordSize()];
    for (int i = 0; i < values_num; i++) {
        const FieldMeta *th_field_meta = table_meta_.GetField(i + table_sys_fields_num);
        int offset = th_field_meta->GetOffset(), len = th_field_meta->GetLen();
        if (th_field_meta->GetAttrType() == AttrType::Chars) {
            int data_len = strlen(static_cast<const char *>(values[i].data));
            if (len > data_len + 1)
                len = data_len + 1;
        }
        memmove(record_data + offset, values[i].data, len);
    }
    return Re::Success;
}
Re IndexInserter::InsertIndex(class Record *record) {
    return index_->InsertEntry(record->GetData(), &record->GetRecordId());
}
static Re InsertIndexRecordReaderAdapter(class Record *record, void *context) {
    IndexInserter &inserter = *reinterpret_cast<IndexInserter *>(context);
    return inserter.InsertIndex(record);
}
Re ScanRecordReaderAdapter(class Record *record, void *context) {
    RecordReaderScanAdapter &adapter = *reinterpret_cast<RecordReaderScanAdapter *>(context);
    adapter.Consume(record);
    return Re::Success;
}
void RecordReaderScanAdapter::Consume(class Record *record) {
    record_reader_(record->GetData(), context_);
}
Re RecordDeleter::DeleteRecord(class Record *record) {
    Re r;
    r = table_.DeleteRecord(txn_, record);
    if (r == Re::Success)
        deleted_count_++;
    return r;
}
