#include "project_operator.h"
#include "../resolve/expression.h"
#include "../resolve/tuple.h"
#include "../storage/field.h"
#include "../storage/table.h"

ProjectOperator::ProjectOperator() {
    tuple_ = new ProjectTuple;
}

ProjectOperator::~ProjectOperator() {
    delete tuple_;
}

void ProjectOperator::AddProjection(const Table *table, const FieldMeta *field) {
    TupleUnitSpec *spec = new TupleUnitSpec(new FieldExpression(table, field));
    spec->SetAlias(field->GetFieldName());
    tuple_->AddUnitSpec(spec);
}

Re ProjectOperator::Init() {
    if (opers_.size() != 1) {
        DebugPrint("ProjectOperator:project operator must has 1 child\n");
        return Re::Internal;
    }
    Re r = opers_[0]->Init();
    if (r != Re::Success) {
        DebugPrint("ProjectOperator:failed to open child operator: %s\n", StrRe(r));
        return r;
    }
    return Re::Success;
}

Re ProjectOperator::Handle() {
    return opers_[0]->Handle();
}

Re ProjectOperator::Destroy() {
    opers_[0]->Destroy();
    return Re::Success;
}

int ProjectOperator::GetTupleUnitsNum() const {
    return tuple_->GetUnitsNum();
}

Re ProjectOperator::GetTupleUnitAt(int index, const TupleUnitSpec *&spec) const {
    return tuple_->GetUnitSpecAt(index, spec);
}

Tuple *ProjectOperator::GetCurrentTuple() {
    tuple_->SetTuple(opers_[0]->GetCurrentTuple());
    return tuple_;
}
