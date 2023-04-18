#pragma once
#include "operator.h"

class IndexScanOperator : public Operator
{
public:
    IndexScanOperator() = default;

    ~IndexScanOperator() override = default;

    Re init() override;

    Re handle() override;

    Re destroy() override;

    Tuple *getCurrentTuple() override;

private:
};