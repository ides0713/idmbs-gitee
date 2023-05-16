#pragma once
#include "../common/re.h"// for Re
#include "operator.h"    // for Operator
class Filter;
class RowTuple;
class Tuple;
///@brief filter predicate for only one table
class PredicateOperator : public Operator
{
public:
    PredicateOperator() : filter_(nullptr) {}
    PredicateOperator(Filter *filter) : filter_(filter) {}
    ~PredicateOperator() override = default;
    Re Init() override;
    Re Handle() override;
    Re Destroy() override;
    Tuple *GetCurrentTuple() override;

private:
    Filter *filter_;

private:
    bool Predicate(RowTuple &row_tuple);
};