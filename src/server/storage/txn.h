#pragma once

#include <atomic>
#include <unordered_set>
#include <unordered_map>
#include <cstddef>
#include "../common/re.h"
#include "../parse/parse_defs.h"

class Table;

struct RecordId;

class Record;

class Operation {
public:
    enum class Type : int {
        Insert,
        Update,
        Delete,
        Undefined
    };
public:
    Operation(Type type, const RecordId &rid);

    [[nodiscard]]Type getType() const { return type_; }

    [[nodiscard]]int32_t getPageId() const { return page_id_; }

    [[nodiscard]]int32_t getSlotId() const { return slot_id_; }

private:
    Type type_;
    int32_t page_id_, slot_id_;
};

class OperationHash {
public:
    size_t operator()(const Operation &op) const {
        return (((size_t) op.getPageId()) << 32) | (op.getSlotId());
    }
};

class OperationPred {
public:
    bool operator()(const Operation &op_1, const Operation &op_2) const {
        return op_1.getPageId() == op_2.getPageId() && op_1.getSlotId() == op_2.getSlotId();
    }
};

class Txn {
public:
    Txn() : txn_id_(0) { start(); }

public:
    void init(Table *table, class Record &rec);

    Re insertRecord(Table *table, class Record *rec);

    Re updateRecord();

    Re deleteRecord();

public:
    static std::atomic<int32_t> txn_id;
public:

    static int32_t getDefaultTxnId();

    static int32_t getNextTxnId();

    static void setTxnId(int32_t id);

    static const char *getTxnFieldName();

    static AttrType getTxnFieldType();

    static int getTxnFieldLen();

private:
    using OperationSet = std::unordered_set<Operation, OperationHash, OperationPred>;
    int32_t txn_id_;
    std::unordered_map<Table *, OperationSet> operations_;
private:
    void start();

    void setRecordTxnId(Table *table, class Record &rec, int32_t txn_id, bool deleted);
};