#pragma once
#include "../common/re.h"// for Re
#include "../resolve/tuple.h"
#include "../storage/record.h"
#include "operator.h"// for Operator
class Table;
class Index;
class IndexScanner;
class IndexScanOperator : public Operator
{
public:
    IndexScanOperator(const Table *table, Index *index, const TupleUnit *left_unit, bool left_inclusive,
                      const TupleUnit *right_unit, bool right_inclusive);
    ~IndexScanOperator() override = default;
    Re Init() override;
    Re Handle() override;
    Re Destroy() override;
    Tuple *GetCurrentTuple() override;

private:
    const Table *table_;
    Index *index_;
    IndexScanner *index_scanner_;
    RecordFileHandler *record_handler_;
    class Record current_record_;
    RowTuple tuple_;
    TupleUnit left_unit_, right_unit_;
    bool left_inclusive_, right_inclusive_;
};