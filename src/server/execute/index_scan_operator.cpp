#include "index_scan_operator.h"
#include "../resolve/tuple.h"
#include "../storage/index.h"
#include "../storage/table.h"
IndexScanOperator::IndexScanOperator(const Table *table, Index *index, const TupleUnit *left_unit, bool left_inclusive,
                                     const TupleUnit *right_unit, bool right_inclusive)
    : table_(nullptr), index_(nullptr), index_scanner_(nullptr), record_handler_(nullptr) {
    if (left_unit != nullptr)
        left_unit_ = *left_unit;
    if (right_unit != nullptr)
        right_unit_ = *right_unit;
}
Re IndexScanOperator::Init() {
    if (table_ == nullptr || index_ == nullptr)
        return Re::Internal;
    IndexScanner *index_scanner =
            index_->CreateScanner(left_unit_.GetData(), left_unit_.GetLength(), left_inclusive_, right_unit_.GetData(),
                                  right_unit_.GetLength(), right_inclusive_);
    if (index_scanner == nullptr) {
        DebugPrint("IndexScanOperator:failed to create index scanner\n");
        return Re::Internal;
    }
    record_handler_ = table_->GetRecordFileHandler();
    if (nullptr == record_handler_) {
        DebugPrint("IndexScanOperator:invalid record handler\n");
        index_scanner->Destroy();
        return Re::Internal;
    }
    index_scanner_ = index_scanner;
    tuple_.SetSchema(table_, table_->GetTableMeta().GetFields());
    return Re::Success;
}
Re IndexScanOperator::Handle() {
    RecordId rid;
    Re r = index_scanner_->NextEntry(&rid);
    if (r != Re::Success)
        return r;
    return record_handler_->GetRecord(&rid, &current_record_);
}
Re IndexScanOperator::Destroy() {
    return Re();
}
Tuple *IndexScanOperator::GetCurrentTuple() {
    return nullptr;
}
