#pragma once
#include "operator.h"

class TupleUnitSpec;
class ProjectTuple;
class FieldMeta;
class Table;
class ProjectOperator : public Operator
{
public:
    ProjectOperator();

    ~ProjectOperator() override = default;

    void addProjection(const Table *table, const FieldMeta *field);

    Re init() override;
    Re handle() override;
    Re destroy() override;

    int getTupleUnitsNum() const;

    Re getTupleUnitAt(int index, const TupleUnitSpec *&spec) const;

    Tuple *getCurrentTuple() override;

private:
    ProjectTuple *tuple_;
};