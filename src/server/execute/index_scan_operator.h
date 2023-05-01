#pragma once
#include "operator.h"                                 // for Operator
#include "../common/re.h"  // for Re

class Tuple;

class IndexScanOperator : public Operator
{
public:
    IndexScanOperator() = default;
    ~IndexScanOperator() override = default;
    Re Init() override;
    Re Handle() override;
    Re Destroy() override;
    Tuple *GetCurrentTuple() override;

private:
};