#include "predicate_operator.h"
#include "../resolve/filter.h"

Re PredicateOperator::init()
{
    if (opers.size() != 1)
    {
        debugPrint("PredicateOperator:predicate operator must has one operator\n");
        return Re::Internal;
    }
    return opers[0]->init();
}

Re PredicateOperator::handle()
{
    Operator *oper = opers[0];
    Re r;
    while ((r = (oper->handle())) == Re::Success)
    {
        Tuple *tuple = oper->getCurrentTuple();
        auto row = static_cast<RowTuple *>(tuple);
        if (tuple == nullptr)
        {
            debugPrint("PredicateOperator:failed to get tuple from operator\n");
            return Re::Internal;
        }
        if (predicate(reinterpret_cast<RowTuple &>(*tuple)))
            return Re::Success;
    }
    return r;
}

Re PredicateOperator::destroy()
{
    opers[0]->destroy();
    return Re::Success;
}

Tuple *PredicateOperator::getCurrentTuple()
{
    Tuple *res = opers[0]->getCurrentTuple();
    return res;
}

bool PredicateOperator::predicate(RowTuple &row_tuple)
{
    const Table &t = row_tuple.getTable();
    if (filter_ == nullptr or filter_->getFilterUnits().empty())
        return true;
    for (const auto *filter_unit : filter_->getFilterUnits())
    {
        Expression *left_expr = filter_unit->getLeft(), *right_expr = filter_unit->getRight();
        CompOp comp = filter_unit->getComp();
        TupleUnit left_unit, right_unit;
        left_expr->getValue(row_tuple, left_unit);
        right_expr->getValue(row_tuple, right_unit);
        const int compare_result = left_unit.compare(right_unit);
        bool filter_result = false;
        switch (comp)
        {
        case CompOp::EqualTo:
            filter_result = (compare_result == 0);
            break;
        case CompOp::NotEqual:
            filter_result = (compare_result != 0);
            break;
        case CompOp::LessThan:
            filter_result = (compare_result < 0);
            break;
        case CompOp::LessEqual:
            filter_result = (compare_result <= 0);
            break;
        case CompOp::GreatThan:
            filter_result = (compare_result > 0);
            break;
        case CompOp::GreatEqual:
            filter_result = (compare_result >= 0);
            break;
        default:
            debugPrint("invalid compare type\n");
            assert(false);
            break;
        }
        if (!filter_result)
            return false;
    }
    return true;
}
