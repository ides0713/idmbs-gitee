#include "txn.h"
#include "table.h"
#include "record.h"

static const uint32_t DELETED_FLAG_BIT_MASK = 0x80000000;
static const uint32_t TXN_ID_BIT_MASK = 0x7FFFFFFF;

std::atomic<int32_t> Txn::global_txn_id(0);

void Txn::init(Table *table, class Record &rec)
{
    setRecordTxnId(table, rec, txn_id_, false);
}

Re Txn::insertRecord(Table *table, struct Record *rec)
{
    Operation *find_operation = findOperation(table, rec->getRecordId());
    if (find_operation != nullptr)
    {
        if (find_operation->getType() == Operation::Type::Delete)
            deleteOperation(table, rec->getRecordId());
        else
            return Re::GenericError;
    }
    insertOperation(table, Operation::Type::Insert, rec->getRecordId());
    return Re::Success;
}

Re Txn::updateRecord(Table *table, class Record *rec)
{
    return Re::GenericError;
}

Re Txn::deleteRecord(Table *table, class Record *rec)
{
    start();
    Operation *find_operation = findOperation(table, rec->getRecordId());
    if (find_operation != nullptr)
    {
        if (find_operation->getType() == Operation::Type::Insert)
        {
            deleteOperation(table, rec->getRecordId());
            return Re::Success;
        }
        else
            return Re::GenericError;
    }
    setRecordTxnId(table, *rec, txn_id_, true);
    insertOperation(table, Operation::Type::Delete, rec->getRecordId());
    return Re::Success;
}

int32_t Txn::getDefaultTxnId()
{
    return 0;
}

int32_t Txn::getNextGlobalTxnId()
{
    return ++global_txn_id;
}

void Txn::setGlobalTxnId(int32_t id)
{
    global_txn_id = id;
}

const char *Txn::getTxnFieldName()
{
    return "__txn";
}

AttrType Txn::getTxnFieldType()
{
    return Ints;
}

int Txn::getTxnFieldLen()
{
    return sizeof(int32_t);
}

void Txn::start()
{
    if (txn_id_ == 0)
        txn_id_ = getNextGlobalTxnId();
}

void Txn::setRecordTxnId(Table *table, class Record &rec, int32_t txn_id, bool deleted)
{
    const FieldMeta *trx_field = table->getTableMeta().getTxnField();
    //    int32_t *txn_id_in_rec = (int32_t *)(rec.getData() + trx_field->getOffset());
    auto txn_id_in_rec = reinterpret_cast<int32_t *>(const_cast<char *>(rec.getData() + trx_field->getOffset()));
    if (deleted)
        txn_id |= DELETED_FLAG_BIT_MASK;
    *txn_id_in_rec = txn_id;
}

void Txn::nextCurrentId()
{
    (void)getNextGlobalTxnId();
    txn_id_ = global_txn_id;
}

Operation *Txn::findOperation(Table *table, const RecordId &record_id)
{
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

void Txn::insertOperation(Table *table, Operation::Type type, const RecordId &rid)
{
    OperationSet &op_set = operations_[table];
    op_set.emplace(type, rid);
}

void Txn::deleteOperation(Table *table, const RecordId &rid)
{
    auto it = operations_.find(table);
    if (it == operations_.end())
        return;
    Operation temp(Operation::Type::Undefined, rid);
    it->second.erase(temp);
}

Operation::Operation(Operation::Type type, const RecordId &rid) : type_(type), page_id_(rid.page_id), slot_id_(rid.slot_id) {}
