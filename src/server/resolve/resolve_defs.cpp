#include "resolve_defs.h"
#include <cassert>
#include "../common/session.h"

void Statement::createStatement(Query *const query, Session *parse_session, Statement *&stmt) {
    switch (query->getScf()) {
        case ScfCreateTable:
            stmt = new CreateTableStatement(query);
            break;
        case ScfInsert:
            stmt = new InsertStatement(query);
            break;
        default:
            debugPrint("Statement:unrecognized query SCF\n");
            break;
    }
}

void SelectStatement::init(Query *query) {}

Re SelectStatement::handle(Query *query, Session *parse_session) { return Re::Success; }

void SelectStatement::destroy() {}

CreateTableStatement::CreateTableStatement(Query *query) :
        Statement(query->getScf()) { table_name_ = nullptr, attr_infos_ = nullptr, attr_infos_num_ = 0; }

void CreateTableStatement::init(Query *query) {
    auto *ctq = static_cast<CreateTableQuery *>(query);
    assert(this->getScf() == ScfCreateTable);
    attr_infos_num_ = ctq->getAttrNum();
    table_name_ = strNew(ctq->getRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = ctq->getAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i] = attr_infos[i];
}

Re CreateTableStatement::handle(Query *query, Session *parse_session) {
    return Re::Success;
}

void CreateTableStatement::destroy() {
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].destroy();
    delete[] attr_infos_;
}

InsertStatement::InsertStatement(Query *query) :
        Statement(query->getScf()), table_name_(nullptr), values_(nullptr), values_num_(0) {}

void InsertStatement::init(Query *query) {
    auto *iq = static_cast<InsertQuery *>(query);
    assert(this->getScf() == ScfInsert);
    values_num_ = iq->getValuesNum();
    table_name_ = strNew(iq->getRelName());
    values_ = new Value[values_num_];
    Value *values = iq->getValues();
    for (int i = 0; i < values_num_; i++)
        values_[i].copy(values[i]);
}

Re InsertStatement::handle(Query *query, Session *parse_session) {
    Txn *txn = new Txn;
    parse_session->setTxn(txn);
    auto *ps = static_cast<ParseSession *>(parse_session);
    DataBase *current_db = ps->getDb();
    if (current_db == nullptr or table_name_ == nullptr or values_num_ <= 0) {
        debugPrint("InsertStatement:invalid argument. db=%p, table_name=%p,values_num=%d\n",
                   current_db, table_name_, values_num_);
        return Re::InvalidArgument;
    }
    std::string table_name_str = std::string(table_name_);
    Table *table = current_db->getTable(table_name_str);
    if (table == nullptr) {
        debugPrint("InsertStatement:no such table. db=%s, table_name=%s\n",
                   current_db->getDbName().c_str(), table_name_);
        return Re::SchemaTableNotExist;
    }
    const TableMeta &table_meta = table->getTableMeta();
    int table_sys_field_num = TableMeta::getSysFieldsNum();
    int table_user_field_num = table_meta.getFieldsNum() - table_sys_field_num;
    //values_num not equal to that in table_meta
    if (values_num_ != table_user_field_num) {
        debugPrint("InsertStatement:field num not equal to values num,values_num=%d,fields_num=%d\n",
                   values_num_, table_user_field_num);
        return Re::SchemaFieldMissing;
    }
    for (int i = 0; i < values_num_; i++) {
        const FieldMeta *field_meta = table_meta.getField(i + table_sys_field_num);
        if (values_[i].type != field_meta->getAttrType()) {
            debugPrint("InsertStatement:field type mismatch. table=%s, field=%s, field type=%d, value_type=%d\n",
                       table_name_, field_meta->getFieldName().c_str(), field_meta->getAttrType(), values_[i].type);
            return Re::SchemaFieldTypeMismatch;
        }
        //todo:advanced data check
        switch (values_[i].type) {
            default:
                break;
        }
    }
    return Re::Success;
}

void InsertStatement::destroy() {
    delete[] table_name_;
    for (int i = 0; i < values_num_; i++)
        values_[i].destroy();
    delete[]values_;
}
