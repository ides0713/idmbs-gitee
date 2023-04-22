#include "predicate_operator.h"
#include "../resolve/filter.h"
Re PredicateOperator::Init() {
    if (opers_.size() != 1) {
        DebugPrint("PredicateOperator:predicate operator must has one operator\n");
        return Re::Internal;
    }
    return opers_[0]->Init();
}
Re PredicateOperator::Handle() {
    Operator *oper = opers_[0];
    Re r;
    while ((r = (oper->Handle())) == Re::Success) {
        Tuple *tuple = oper->GetCurrentTuple();
        auto row = static_cast<RowTuple *>(tuple);
        if (tuple == nullptr) {
            DebugPrint("PredicateOperator:failed to get tuple from operator\n");
            return Re::Internal;
        }
        if (Predicate(reinterpret_cast<RowTuple &>(*tuple)))
            return Re::Success;
    }
    return r;
}
Re PredicateOperator::Destroy() {
    opers_[0]->Destroy();
    return Re::Success;
}
Tuple *PredicateOperator::GetCurrentTuple() {
    Tuple *res = opers_[0]->GetCurrentTuple();
    return res;
}
bool PredicateOperator::Predicate(RowTuple &row_tuple) {
    const Table &t = row_tuple.GetTable();
    if (filter_ == nullptr or filter_->GetFilterUnits().empty())
        return true;
    for (const auto *filter_unit: filter_->GetFilterUnits()) {
        Expression *left_expr = filter_unit->GetLeft(), *right_expr = filter_unit->GetRight();
        CompOp comp = filter_unit->GetComp();
        TupleUnit left_unit, right_unit;
        left_expr->GetValue(row_tuple, left_unit);
        right_expr->GetValue(row_tuple, right_unit);
        const int compare_result = left_unit.Compare(right_unit);
        bool filter_result = false;
        switch (comp) {
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
                DebugPrint("invalid compare type\n");
                assert(false);
                break;
        }
        if (!filter_result)
            return false;
    }
    return true;
}
