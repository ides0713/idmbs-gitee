#pragma once
#include "../common/re.h"
#include <vector>

class Tuple;

class Operator {
public:
    Operator() = default;

    virtual ~Operator() = default;

    virtual Re Init() = 0;

    virtual Re Handle() = 0;

    virtual Re Destroy() = 0;

    virtual Tuple *GetCurrentTuple() = 0;

    void AddOper(Operator *oper);

protected:
    std::vector<Operator *> opers_;
};