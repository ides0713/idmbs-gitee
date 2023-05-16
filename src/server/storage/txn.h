#pragma once
#include "../common/re.h"       // for Re
#include "../parse/parse_defs.h"// for AttrType
#include <atomic>               // for atomic
#include <cstddef>              // for size_t
#include <stdint.h>             // for int32_t
#include <unordered_map>        // for unordered_map
#include <unordered_set>        // for unordered_set
class Table;
struct RecordId;
class Operation
{
public:
    enum class Type : int
    {
        Insert,
        Update,
        Delete,
        Undefined
    };

public:
    Operation(Type type, const RecordId &rid);
    [[nodiscard]] Type GetType() const { return type_; }
    [[nodiscard]] int32_t GetPageId() const { return page_id_; }
    [[nodiscard]] int32_t GetSlotId() const { return slot_id_; }

private:
    Type type_;
    int32_t page_id_, slot_id_;
};
class OperationHash
{
public:
    size_t operator()(const Operation &op) const { return (((size_t) op.GetPageId()) << 32) | (op.GetSlotId()); }
};
class OperationPred
{
public:
    bool operator()(const Operation &op_1, const Operation &op_2) const {
        return op_1.GetPageId() == op_2.GetPageId() && op_1.GetSlotId() == op_2.GetSlotId();
    }
};
class Txn
{
public:
    Txn() : txn_id_(0) { Start(); }

public:
    void Init(Table *table, class Record &rec);
    Re InsertRecord(Table *table, class Record *rec);
    Re UpdateRecord(Table *table, class Record *rec);
    Re DeleteRecord(Table *table, class Record *rec);
    int32_t GetTxnId() { return txn_id_; }
    void SetTxnId(int32_t txn_id) { txn_id_ = txn_id; }
    void NextCurrentId();
    bool IsVisible(Table *table, class Record *rec);

public:
    static std::atomic<int32_t> global_txn_id;

public:
    static int32_t GetDefaultTxnId();
    static int32_t GetNextGlobalTxnId();
    static void SetGlobalTxnId(int32_t id);
    static const char *GetTxnFieldName();
    static AttrType GetTxnFieldType();
    static int GetTxnFieldLen();

private:
    using OperationSet = std::unordered_set<Operation, OperationHash, OperationPred>;
    int32_t txn_id_;
    std::unordered_map<Table *, OperationSet> operations_;

private:
    void Start();
    void SetRecordTxnId(Table *table, class Record &rec, int32_t txn_id, bool deleted);
    Operation *FindOperation(Table *table, const RecordId &record_id);
    void InsertOperation(Table *table, Operation::Type type, const RecordId &rid);
    void DeleteOperation(Table *table, const RecordId &rid);

private:
    static void GetRecordTxnId(Table *table, class Record &record, int32_t &txn_id, bool &deleted);
};