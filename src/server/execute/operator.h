#include "../common/re.h"
#include <vector>

class Tuple;

class Operator {
public:
    Operator() = default;

    virtual ~Operator() = default;

    virtual Re init() = 0;

    virtual Re handle() = 0;

    virtual Re destroy() = 0;

    virtual Tuple *getCurrentTuple() = 0;

    void addOper(Operator *oper);

protected:
    std::vector<Operator *> opers;
};