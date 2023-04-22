#pragma once
#include "operator.h"
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