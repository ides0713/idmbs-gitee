#include "txn.h"
#include "table.h"
#include "record.h"

static const uint32_t DELETED_FLAG_BIT_MASK = 0x80000000;
static const uint32_t TXN_ID_BIT_MASK = 0x7FFFFFFF;

std::atomic<int32_t> Txn::txn_id(0);

void Txn::init(Table *table, class Record &rec) {
    setRecordTxnId(table, rec, txn_id_, false);
}

Re Txn::insertRecord(Table *table, struct Record *rec) {
    return RecordEof;
}

int32_t Txn::getDefaultTxnId() {
    return 0;
}

int32_t Txn::getNextTxnId() {
    return ++txn_id;
}

void Txn::setTxnId(int32_t id) {
    txn_id = id;
}

const char *Txn::getTxnFieldName() {
    return "__txn";
}

AttrType Txn::getTxnFieldType() {
    return Ints;
}

int Txn::getTxnFieldLen() {
    return sizeof(int32_t);
}

void Txn::start() {
    if (txn_id_ == 0)
        txn_id_ = getNextTxnId();
}

void Txn::setRecordTxnId(Table *table, class Record &rec, int32_t txn_id, bool deleted) {
    const FieldMeta *trx_field = table->getTableMeta().getTxnField();
    int32_t *txn_id_in_rec = (int32_t *) (rec.getData() + trx_field->getOffset());
    if (deleted) {
        txn_id |= DELETED_FLAG_BIT_MASK;
    }
    *txn_id_in_rec = txn_id;
}

void Txn::nextCurrentId() {
    (void) getNextTxnId();
    txn_id_ = txn_id;
}

Operation::Operation(Operation::Type type, const RecordId &rid) :
        type_(type), page_id_(rid.page_id), slot_id_(rid.slot_id) {}
