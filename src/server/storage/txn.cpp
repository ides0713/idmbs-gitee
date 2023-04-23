#include "txn.h"

#include <utility>                                           // for pair

#include "record.h"                                          // for Record
#include "table.h"                                           // for Table
#include "/home/ubuntu/idbms/src/server/common/re.h"         // for Re, Gene...
#include "/home/ubuntu/idbms/src/server/parse/parse_defs.h"  // for AttrType
#include "field.h"                                           // for FieldMeta

static const uint32_t DELETED_FLAG_BIT_MASK = 0x80000000;
static const uint32_t TXN_ID_BIT_MASK = 0x7FFFFFFF;
std::atomic<int32_t> Txn::global_txn_id(0);
void Txn::Init(Table *table, class Record &rec) {
    SetRecordTxnId(table, rec, txn_id_, false);
}
Re Txn::InsertRecord(Table *table, struct Record *rec) {
    Operation *find_operation = FindOperation(table, rec->GetRecordId());
    if (find_operation != nullptr) {
        if (find_operation->GetType() == Operation::Type::Delete)
            DeleteOperation(table, rec->GetRecordId());
        else
            return Re::GenericError;
    }
    InsertOperation(table, Operation::Type::Insert, rec->GetRecordId());
    return Re::Success;
}
Re Txn::UpdateRecord(Table *table, class Record *rec) {
    return Re::GenericError;
}
Re Txn::DeleteRecord(Table *table, class Record *rec) {
    Start();
    Operation *find_operation = FindOperation(table, rec->GetRecordId());
    if (find_operation != nullptr) {
        if (find_operation->GetType() == Operation::Type::Insert) {
            DeleteOperation(table, rec->GetRecordId());
            return Re::Success;
        } else
            return Re::GenericError;
    }
    SetRecordTxnId(table, *rec, txn_id_, true);
    InsertOperation(table, Operation::Type::Delete, rec->GetRecordId());
    return Re::Success;
}
int32_t Txn::GetDefaultTxnId() {
    return 0;
}
int32_t Txn::GetNextGlobalTxnId() {
    return ++global_txn_id;
}
void Txn::SetGlobalTxnId(int32_t id) {
    global_txn_id = id;
}
const char *Txn::GetTxnFieldName() {
    return "__txn";
}
AttrType Txn::GetTxnFieldType() {
    return Ints;
}
int Txn::GetTxnFieldLen() {
    return sizeof(int32_t);
}
void Txn::Start() {
    if (txn_id_ == 0)
        txn_id_ = GetNextGlobalTxnId();
}
void Txn::SetRecordTxnId(Table *table, class Record &rec, int32_t txn_id, bool deleted) {
    const FieldMeta *trx_field = table->GetTableMeta().GetTxnField();
    //    int32_t *txn_id_in_rec = (int32_t *)(rec.getData() + trx_field->getOffset());
    auto txn_id_in_rec = reinterpret_cast<int32_t *>(const_cast<char *>(rec.GetData() + trx_field->GetOffset()));
    if (deleted)
        txn_id |= DELETED_FLAG_BIT_MASK;
    *txn_id_in_rec = txn_id;
}
void Txn::NextCurrentId() {
    (void) GetNextGlobalTxnId();
    txn_id_ = global_txn_id;
}
Operation *Txn::FindOperation(Table *table, const RecordId &record_id) {
    auto set_it = operations_.find(table);
    if (set_it == operations_.end())
        return nullptr;
    OperationSet &table_operations_set = set_it->second;
    Operation temp(Operation::Type::Undefined, record_id);
    auto op_it = table_operations_set.find(temp);
    if (op_it == table_operations_set.end())
        return nullptr;
    auto res = const_cast<Operation *>(&(*op_it));
    return res;
}
void Txn::InsertOperation(Table *table, Operation::Type type, const RecordId &rid) {
    OperationSet &op_set = operations_[table];
    op_set.emplace(type, rid);
}
void Txn::DeleteOperation(Table *table, const RecordId &rid) {
    auto it = operations_.find(table);
    if (it == operations_.end())
        return;
    Operation temp(Operation::Type::Undefined, rid);
    it->second.erase(temp);
}
Operation::Operation(Operation::Type type, const RecordId &rid)
    : type_(type), page_id_(rid.page_id), slot_id_(rid.slot_id) {
}
