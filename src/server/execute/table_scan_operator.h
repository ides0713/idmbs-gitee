#pragma once

#include "operator.h"
#include "../storage/record.h"
#include "../common/re.h"
#include "../resolve/tuple.h"

class Table;

class TableScanOperator : public Operator {
public:
    TableScanOperator(Table *table) : table_(table) {}

    ~TableScanOperator() override = default;

    Re init() override;

    Re handle() override;

    Re destroy() override;

    Tuple *getCurrentTuple() override;

private:
    Table *table_ = nullptr;
    RecordFileScanner record_scanner_;
    class Record current_record_;
    RowTuple tuple_;
};