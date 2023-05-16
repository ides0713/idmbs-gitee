#include "table_scan_operator.h"
#include "../common/re.h"     // for Re, RecordEof
#include "../resolve/tuple.h" // for RowTuple
#include "../storage/record.h"// for RecordFile...
#include "../storage/table.h" // for Table, Tab...
Re TableScanOperator::Init() {
    Re r = table_->GetRecordFileScanner(record_scanner_);
    if (r == Re ::Success)
        tuple_.SetSchema(table_, table_->GetTableMeta().GetFields());
    return r;
}
Re TableScanOperator::Handle() {
    if (!record_scanner_.HasNext())
        return Re::RecordEof;
    return record_scanner_.Next(current_record_);
}
Re TableScanOperator::Destroy() {
    return record_scanner_.Destroy();
}
Tuple *TableScanOperator::GetCurrentTuple() {
    tuple_.SetRecord(&current_record_);
    return &tuple_;
}
