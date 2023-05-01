#include "delete_operator.h"

#include <vector>                                         // for vector

#include "../../common/common_defs.h"                     // for DebugPrint
#include "../resolve/resolve_defs.h"                      // for DeleteState...
#include "../storage/table.h"                             // for Table
#include "../resolve/tuple.h"  // for RowTuple

Re DeleteOperator::Init() {
    if (opers_.size() != 1) {
        DebugPrint("DeleteOperator:delete operaot must has 1 child operator\n");
        return Re::Internal;
    }
    Operator *child_oper = opers_[0];
    Re r = opers_[0]->Init();
    if (r != Re::Success) {
        DebugPrint("DeleteOperator:failed to init child operator re:%d,%s\n", r, StrRe(r));
        return r;
    }
    return Re::Success;
}
Re DeleteOperator::Handle() {
    Table *table = delete_stmt_->GetTable();
    Re r;
    Operator *oper = opers_[0];
    while ((r = oper->Handle()) == Re::Success) {
        Tuple *tuple = oper->GetCurrentTuple();
        if (tuple == nullptr) {
            DebugPrint("DeleteOperator:failed to get current tuple,re:%d,%s", r, StrRe(r));
            return r;
        }
        auto row_tuple = static_cast<RowTuple *>(tuple);
        class Record &record = row_tuple->GetRecord();
        r = table->DeleteRecord(txn_, &record);
        if (r != Re::Success) {
            DebugPrint("DeleteOperator:failed to delete record re:%d,%s", r, StrRe(r));
            return r;
        }
    }
    return Re::Success;
}
Re DeleteOperator::Destroy() {
    opers_[0]->Destroy();
    return Re::Success;
}
