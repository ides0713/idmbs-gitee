#include "project_operator.h"
#include "../resolve/tuple.h"
#include "../storage/table.h"
#include "../storage/field.h"
ProjectOperator::ProjectOperator()
{
    tuple_ = new ProjectTuple;
}

Re ProjectOperator::init()
{
    if (opers.size() != 1)
    {
        debugPrint("ProjectOperator:project operator must has 1 child\n");
        return Re::Internal;
    }
    Operator *oper_1 = opers[0];
    Re r = oper_1->init();
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
