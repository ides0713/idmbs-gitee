#include "txn.h"
#include "table.h"

std::atomic<int32_t> Txn::txn_id(0);

void Txn::start() {
    if (txn_id_ == 0)
        txn_id_ = getNextTxnId();
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