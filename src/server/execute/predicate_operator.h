#pragma once

#include "operator.h"

class Filter;

class RowTuple;

///@brief filter predicate for only one table
class PredicateOperator : public Operator {
public:
    PredicateOperator() : filter_(nullptr) {}

    PredicateOperator(Filter *filter) : filter_(filter) {}

    ~PredicateOperator() override = default;

    Re init() override;

    Re handle() override;

    Re destroy() override;

    Tuple *getCurrentTuple() override;

private:
    Filter *filter_;
private:
    bool predicate(RowTuple &row_tuple);
};