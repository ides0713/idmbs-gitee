#include "project_operator.h"
#include "../resolve/tuple.h"
#include "../storage/table.h"
#include "../storage/field.h"
#include "../resolve/expression.h"

ProjectOperator::ProjectOperator()
{
    tuple_ = new ProjectTuple;
}

ProjectOperator::~ProjectOperator()
{
    delete tuple_;
}

void ProjectOperator::addProjection(const Table *table, const FieldMeta *field)
{
    TupleUnitSpec *spec = new TupleUnitSpec(new FieldExpression(table, field));
    spec->setAlias(field->getFieldName());
    tuple_->addUnitSpec(spec);
}

Re ProjectOperator::init()
{
    if (opers.size() != 1)
    {
        debugPrint("ProjectOperator:project operator must has 1 child\n");
        return Re::Internal;
    }
    Re r = opers[0]->init();
    if (r != Re::Success)
    {
        debugPrint("ProjectOperator:failed to open child operator: %s\n", strRe(r));
        return r;
    }
    return Re::Success;
}

Re ProjectOperator::handle()
{
    return opers[0]->handle();
}

Re ProjectOperator::destroy()
{
    opers[0]->destroy();
    return Re::Success;
}

int ProjectOperator::getTupleUnitsNum() const
{
    return tuple_->getUnitsNum();
}

Re ProjectOperator::getTupleUnitAt(int index, const TupleUnitSpec *&spec) const
{
    return tuple_->getUnitSpecAt(index, spec);
}

Tuple *ProjectOperator::getCurrentTuple()
{
    tuple_->setTuple(opers[0]->getCurrentTuple());
    return tuple_;
}
