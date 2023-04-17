#include "table_scan_operator.h"
#include "../storage/table.h"
#include "../resolve/tuple.h"

Re TableScanOperator::init() {
//    Re r = table_->ge(record_scanner_);
    if (r == Re ::Success) {
        tuple_.set_schema(table_, table_->table_meta().field_metas());
    }
    return rc;
}

Re TableScanOperator::handle() {
    return RecordEof;
}

Re TableScanOperator::destroy() {
    return RecordEof;
}

Tuple *TableScanOperator::getCurrentTuple() {
    return nullptr;
}
