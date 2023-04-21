#pragma once

#include "../common/re.h"
#include "../resolve/tuple.h"
#include "../storage/record.h"
#include "operator.h"

class Table;

class TableScanOperator : public Operator {
public:
    TableScanOperator() : table_(nullptr) {}
    TableScanOperator(Table *table) : table_(table) {}

    ~TableScanOperator() override = default;

    Re Init() override;

    Re Handle() override;

    Re Destroy() override;

    Tuple *GetCurrentTuple() override;

private:
    Table *table_;
    RecordFileScanner record_scanner_;
    class Record current_record_;
    RowTuple tuple_;
};