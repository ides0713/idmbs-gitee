#pragma once
#include "operator.h"                                 // for Operator
#include "../common/re.h"  // for Re

class TupleUnitSpec;
class ProjectTuple;
class FieldMeta;
class Table;
class Tuple;

class ProjectOperator : public Operator
{
public:
    ProjectOperator();
    ~ProjectOperator() override;
    void AddProjection(const Table *table, const FieldMeta *field);
    Re Init() override;
    Re Handle() override;
    Re Destroy() override;
    int GetTupleUnitsNum() const;
    Re GetTupleUnitAt(int index, const TupleUnitSpec *&spec) const;
    Tuple *GetCurrentTuple() override;

private:
    ProjectTuple *tuple_;
};