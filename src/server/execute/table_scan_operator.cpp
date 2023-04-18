#include "table_scan_operator.h"
#include "../storage/table.h"
#include "../resolve/tuple.h"

Re TableScanOperator::init()
{
    Re r = table_->getRecordFileScanner(record_scanner_);
    if (r == Re ::Success)
        tuple_.setSchema(table_, table_->getTableMeta().getFields());
    return r;
}

Re TableScanOperator::handle()
{
    if (!record_scanner_.hasNext())
        return Re::RecordEof;
    return record_scanner_.next(current_record_);
}

Re TableScanOperator::destroy()
{
    return record_scanner_.destroy();
}

Tuple *TableScanOperator::getCurrentTuple()
{
    tuple_.setRecord(&current_record_);
    return &tuple_;
}
