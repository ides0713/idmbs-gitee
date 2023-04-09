#include "table.h"
#include <utility>
#include "txn.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include "../common/global_managers.h"
#include "storage_defs.h"
#include "storage_handler.h"

static const Json::StaticString FIELD_TABLE_NAME("table_name");
static const Json::StaticString FIELD_FIELDS("fields");
static const Json::StaticString FIELD_INDEXES("indexes");

std::vector<FieldMeta> TableMeta::sys_fields_;

TableMeta::TableMeta(const TableMeta &other)
        : table_name_(other.table_name_), fields_(other.fields_), indexes_(other.indexes_),
          record_size_(other.record_size_) {}

Re TableMeta::init(const char *table_name, int32_t attr_infos_num, const AttrInfo *attr_infos) {
    if (strlen(table_name) == 0 or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        debugPrint("TableMeta:init table_meta:%s failed,table name:%s is not valid\n", table_name, table_name);
        return Re::InvalidArgument;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        debugPrint("TableMeta:init table_meta:%s failed,attr is not valid\n", table_name);
        return Re::InvalidArgument;
    }
    if (sys_fields_.empty()) {
        Re r = initializeSysFields();
        if (r != Re::Success) {
            debugPrint("TableMeta:init table_meta:%s failed,init system fields failed\n", table_name);
            return r;
        }
    }
    int sys_fields_size = sys_fields_.size();
    fields_.resize(attr_infos_num + sys_fields_size);
    for (size_t i = 0; i < sys_fields_size; i++)
        fields_[i] = sys_fields_[i];
    int field_offset = sys_fields_.back().getOffset() + sys_fields_.back().getLen();
    for (int i = 0; i < attr_infos_num; i++) {
        const AttrInfo &attr_info = attr_infos[i];
        Re r = fields_[i + sys_fields_size].init(attr_info.attr_name, attr_info.attr_type, field_offset,
                                                 attr_info.attr_len, true);
        if (r != Re::Success) {
            debugPrint("TableMeta:Failed to init field meta.table name=%s,getFieldName name:%s\n", table_name,
                       attr_info.attr_name);
            return r;
        }
        field_offset += attr_info.attr_len;
    }
    record_size_ = field_offset, table_name_ = table_name;
    debugPrint("TableMeta:initialized table meta.table name=%s\n", table_name);
    return Re::Success;
}

Re TableMeta::initializeSysFields() {
    sys_fields_.reserve(1);
    FieldMeta field_meta;
    Re r = field_meta.init(Txn::getTxnFieldName(), Txn::getTxnFieldType(), 0, Txn::getTxnFieldLen(), false);
    if (r != Re::Success) {
        debugPrint("TableMeta:failed to init system fields\n");
        return r;
    }
    sys_fields_.push_back(field_meta);
    return Re::Success;
}

int TableMeta::serialize(std::ostream &ostream) const {
    Json::Value table_value;
    table_value[FIELD_TABLE_NAME] = table_name_;

    Json::Value fields_value;
    for (const FieldMeta &field: fields_) {
        Json::Value field_value;
        field.toJson(field_value);
        fields_value.append(std::move(field_value));
    }
    table_value[FIELD_FIELDS] = std::move(fields_value);
    Json::Value indexes_value;
    for (const auto &index: indexes_) {
        Json::Value index_value;
        index.toJson(index_value);
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

int TableMeta::deserialize(std::istream &istream) {
    if (sys_fields_.empty()) {
        initializeSysFields();
    }
    Json::Value table_value;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::streampos old_pos = istream.tellg();
    if (!Json::parseFromStream(builder, istream, &table_value, &errors)) {
        debugPrint("TableMeta:failed to deserialize table meta. error=%s\n", errors.c_str());
        return -1;
    }

    const Json::Value &table_name_value = table_value[FIELD_TABLE_NAME];
    if (!table_name_value.isString()) {
        debugPrint("TableMeta:invalid table name. json value=%s\n", table_name_value.toStyledString().c_str());
        return -1;
    }

    std::string table_name = table_name_value.asString();

    const Json::Value &fields_value = table_value[FIELD_FIELDS];
    if (!fields_value.isArray() or fields_value.size() <= 0) {
        debugPrint("TableMeta:invalid table meta. fields is not array, json value=%s\n",
                   fields_value.toStyledString().c_str());
        return -1;
    }

    int field_num = fields_value.size();
    std::vector<FieldMeta> fields(field_num);
    for (int i = 0; i < field_num; i++) {
        FieldMeta &field = fields[i];

        const Json::Value &field_value = fields_value[i];
        Re r = FieldMeta::fromJson(field_value, field);
        if (r != Re::Success) {
            debugPrint("TableMeta:failed to deserialize table meta. table name =%s\n", table_name.c_str());
            return -1;
        }
    }

    std::sort(
            fields.begin(), fields.end(),
            [](const FieldMeta &f_1, const FieldMeta &f_2) { return f_1.getOffset() < f_2.getOffset(); });

    table_name_.swap(table_name);
    fields_.swap(fields);
    record_size_ = fields_.back().getOffset() + fields_.back().getLen() - fields_.begin()->getOffset();

    const Json::Value &indexes_value = table_value[FIELD_INDEXES];
    if (!indexes_value.empty()) {
        if (!indexes_value.isArray()) {
            debugPrint("TableMeta:invalid table meta. indexes is not array, json value=%s\n",
                       fields_value.toStyledString().c_str());
            return -1;
        }
        const int index_num = indexes_value.size();
        std::vector<IndexMeta> indexes(index_num);
        for (int i = 0; i < index_num; i++) {
            IndexMeta &index = indexes[i];
            const Json::Value &index_value = indexes_value[i];
            Re r = IndexMeta::fromJson(*this, index_value, index);
            if (r != Re::Success) {
                debugPrint("TableMeta:failed to deserialize table meta. table name=%s\n", table_name.c_str());
                return -1;
            }
        }
        indexes_.swap(indexes);
    }
    return (int) (istream.tellg() - old_pos);
}

int TableMeta::getSerialSize() const {
    return -1;
}

const FieldMeta *TableMeta::getField(int index) const {
    return &fields_[index];
}

const FieldMeta *TableMeta::getField(const char *field_name) const {
    if (strlen(field_name) == 0) {
        debugPrint("TableMeta:getFrame field:%s failed,field_name invalid\n", field_name);
        return nullptr;
    }
    for (const FieldMeta &field: fields_)
        if (strcmp(field.getFieldName().c_str(), field_name) == 0)
            return &field;
    return nullptr;
}

const FieldMeta *TableMeta::getField(std::string field_name) const {
    return getField(field_name.c_str());
}

int TableMeta::getSysFieldsNum() {
    return sys_fields_.size();
}

const FieldMeta *TableMeta::getTxnField() const {
    return &fields_[0];
}

Re Table::init(std::filesystem::path database_path, const char *table_name, const size_t attr_infos_num,
               const AttrInfo *attr_infos, ClogManager *clog_manager) {
    if (strlen(table_name) == 0 or strlen(table_name) > TABLE_NAME_MAX_LEN) {
        debugPrint("Table:init table:%s failed,table getTableName:%s is not valid \n", table_name, table_name);
        return Re::InvalidArgument;
    }
    if (attr_infos == nullptr or attr_infos_num <= 0) {
        debugPrint("Table:init table:%s failed,attr is not valid\n", table_name);
        return Re::InvalidArgument;
    }
    namespace fs = std::filesystem;
    fs::path table_meta_file_path = getTableMetaFilePath(database_path, table_name);
    FILE *f = fopen(table_meta_file_path.c_str(), "r");
    if (f != nullptr) {
        debugPrint("Table:init failed,table:%s already exists\n", table_name);
        fclose(f);
        return Re::SchemaTableExist;
    }
    f = fopen(table_meta_file_path.c_str(), "a");
    fclose(f);
    Re r = table_meta_.init(table_name, attr_infos_num, attr_infos);
    if (r != Re::Success) {
        debugPrint("Table:init failed,table:%s\n", table_name);
        return r;
    }
    std::fstream file_stream;
    file_stream.open(table_meta_file_path, std::ios_base::out | std::ios_base::binary);
    if (!file_stream.is_open()) {
        debugPrint("Table:Failed to open file for write. file name=%s, errmsg=%s\n", table_meta_file_path.c_str(),
                   strerror(errno));
        return Re::IoErr;
    }
    table_meta_.serialize(file_stream);
    file_stream.close();

    std::string data_file_name = std::string(table_name) + ".data";
    fs::path data_file_path = fs::path(database_path).append(data_file_name);
    GlobalBufferPoolManager &bpm = GlobalManagers::globalBufferPoolManager();
    r = bpm.createFile(data_file_path.c_str());
    if (r != Re::Success) {
        debugPrint("Table:failed to create disk buffer pool of data file. file name=%s\n", data_file_name.c_str());
        return r;
    }
    r = initRecordHandler(database_path);
    if (r != Re::Success) {
        debugPrint("Table:failed to create table %s due to init record handler failed.", data_file_name.c_str());
        // don't need to remove the data_file
        return r;
    }
    database_path_ = database_path;
    clog_manager_ = clog_manager;
    debugPrint("Table:successfully create table %s:%s", database_path_.c_str(), table_meta_.getTableName().c_str());
    return Re::Success;
}

Re Table::initRecordHandler(const char *base_dir) {
    namespace fs = std::filesystem;
    std::string data_file_name = table_meta_.getTableName() + ".data";
    fs::path data_file_path = fs::path(base_dir).append(data_file_name);
    GlobalBufferPoolManager &bpm = GlobalManagers::globalBufferPoolManager();
    Re r = bpm.openFile(data_file_path, data_buffer_pool_);
    if (r != Re::Success) {
        debugPrint("TableMeta:failed to open disk buffer pool for file:%s. rc=%d\n", data_file_name.c_str(), r);
        return r;
    }
    record_handler_ = new RecordFileHandler();
    r = record_handler_->init(data_buffer_pool_);
    if (r != Re::Success) {
        debugPrint("TableMeta:failed to init record handler. rc=%d:%s\n", r, strRe(r));
        bpm.closeFile(data_file_path);
        data_buffer_pool_ = nullptr;
        delete record_handler_;
        record_handler_ = nullptr;
        return r;
    }
    return Re::Success;
}

Re Table::initRecordHandler(std::filesystem::path base_dir) {
    return initRecordHandler(base_dir.c_str());
}

Re Table::init(std::filesystem::path database_path, const char *table_name, ClogManager *clog_manager) {
    namespace fs = std::filesystem;
    std::fstream f_stream;
    fs::path table_meta_file_path = getTableMetaFilePath(database_path, table_name);
    f_stream.open(table_meta_file_path.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!f_stream.is_open()) {
        debugPrint("Table:failed to open meta file for read. file name=%s, errmsg=%s\n",
                   table_meta_file_path.c_str(), strerror(errno));
        return Re::IoErr;
    }
    if (table_meta_.deserialize(f_stream) < 0) {
        debugPrint("Table:failed to deserialize table meta. file name=%s\n",
                   table_meta_file_path.c_str());
        f_stream.close();
        return Re::GenericError;
    }
    f_stream.close();
    Re r = initRecordHandler(database_path);
    if (r != Re::Success) {
        debugPrint("Table:failed to open table %s due to init record handler failed.\n",
                   table_meta_.getTableName().c_str());
        // don't need to remove the data_file
        return r;
    }
    if (clog_manager_ == nullptr)
        clog_manager_ = clog_manager;
    return Re::Success;
}

Re Table::insertRecord(Txn *txn, int values_num, const Value *values) {
    char *record_data;
    makeRecord(values_num, values, record_data);
    class Record *rec = new class Record;
    rec->setData(record_data);
    Re r = insertRecord(txn, rec);
    delete[]record_data;
    return r;
}

Re Table::insertRecord(Txn *txn, class Record *rec) {
    if (txn != nullptr)
        txn->init(this, *rec);
    Re r = record_handler_->insertRecord(rec->getData(), table_meta_.getRecordSize(), &rec->getRecordId());
    if (r != Re::Success) {
        debugPrint("Table:insert record failed. table name=%s, re=%d:%s\n",
                   table_meta_.getTableName().c_str(), r, strRe(r));
        return r;
    }
    if (txn != nullptr) {
        r = txn->insertRecord(this,rec);
        if (r != Re::Success) {
            LOG_ERROR("Failed to log operation(insertion) to trx");

            RC rc2 = record_handler_->delete_record(&record->rid());
            if (rc2 != RC::SUCCESS) {
                LOG_ERROR("Failed to rollback record data when insert index entries failed. table name=%s, rc=%d:%s",
                          name(),
                          rc2,
                          strrc(rc2));
            }
            return rc;
        }
    }
    return Re::Success;
}

Re Table::makeRecord(int values_num, const Value *values, char *&record_data) {
    int table_sys_fields_num = table_meta_.getSysFieldsNum();
    if (values_num != table_meta_.getFieldsNum() - table_sys_fields_num) {
        debugPrint("Table:make record failed,values_num not match,values num=%d,table's common fields num=%d\n",
                   values_num, table_meta_.getFieldsNum() - table_sys_fields_num);
        return Re::SchemaFieldMissing;
    }
    //type check before copy the
    for (int i = 0; i < values_num; i++) {
        const Value &th_value = values[i];
        const FieldMeta *th_field_meta = table_meta_.getField(i + table_sys_fields_num);
        if (th_value.type != th_field_meta->getAttrType()) {
            debugPrint("Table:make record failed,values type %d mismatch fields type %d\n",
                       th_value.type, th_field_meta->getAttrType());
            return Re::SchemaFieldTypeMismatch;
        }
    }
    record_data = new char[table_meta_.getRecordSize()];
    for (int i = 0; i < values_num; i++) {
        const FieldMeta *th_field_meta = table_meta_.getField(i + table_sys_fields_num);
        int offset = th_field_meta->getOffset(), len = th_field_meta->getLen();
        if (th_field_meta->getAttrType() == AttrType::Chars) {
            int data_len = strlen(static_cast<const char *>(values[i].data));
            if (len > data_len + 1)
                len = data_len + 1;
        }
        memcpy(record_data + offset, values[i].data, len);
    }
    return Re::Success;
}
