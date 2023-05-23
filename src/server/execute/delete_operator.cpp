#include "delete_operator.h"
#include "../../common/common_defs.h"// for DebugPrint
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"
#include "../resolve/resolve_defs.h"// for DeleteState...
#include "../resolve/tuple.h"       // for RowTuple
#include "../storage/table.h"       // for Table
#include <vector>                   // for vector
Re DeleteOperator::Init() {
    if (opers_.size() != 1) {
        DebugPrint("DeleteOperator:delete operaot must has 1 child operator\n");
        return Re::Internal;
    }
    Operator *child_oper = opers_[0];
    Re r = opers_[0]->Init();
    if (r != Re::Success) {
        DebugPrint("DeleteOperator:failed to init child operator r:%d,%s\n", r, StrRe(r));
        return r;
    }
    return Re::Success;
}
Re DeleteOperator::Handle() {
    Table *table = delete_stmt_->GetTable();
    Re r;
    int delete_count = 0;
    Operator *oper = opers_[0];
    while ((r = oper->Handle()) == Re::Success) {
        Tuple *tuple = oper->GetCurrentTuple();
        if (tuple == nullptr) {
            DebugPrint("DeleteOperator:failed to get current tuple,r:%d,%s", r, StrRe(r));
            return r;
        }
        auto row_tuple = static_cast<RowTuple *>(tuple);
        class Record &record = row_tuple->GetRecord();
        r = table->DeleteRecord(txn_, &record);
        if (r != Re::Success) {
            DebugPrint("DeleteOperator:failed to delete record r:%d,%s", r, StrRe(r));
            return r;
        }
        delete_count++;
    }
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    gmm.SetResponse("DELETE SUCCEEDED,DELETE %d RECORDS FROM TABLE.\n", delete_count);
    return Re::Success;
}
Re DeleteOperator::Destroy() {
    opers_[0]->Destroy();
    return Re::Success;
}
