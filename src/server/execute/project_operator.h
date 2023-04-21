#pragma once
#include "operator.h"

class TupleUnitSpec;
class ProjectTuple;
class FieldMeta;
class Table;
class ProjectOperator : public Operator {
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