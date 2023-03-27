#include "execute_main.h"
#include <cassert>

Re ExecuteMain::handle() {
    ResolveSession *rs = static_cast<ResolveSession *>(resolve_session_);
    Statement *stmt = rs->getStmt();
    assert(stmt != nullptr);
    switch (stmt->getType()) {
        case StatementFlag::Select:
            doSelect(stmt);
            break;
        case StatementFlag::CreateTable:
            doCreateTable(stmt);
            break;
        default:
            break;
    }
    return Re::Success;
}

Session *ExecuteMain::callBack() {
    return execute_session_;
}

Re ExecuteMain::doSelect(Statement *stmt) {
    SelectStatement *s = static_cast<SelectStatement *>(stmt);

    return Re::Success;
}

Re ExecuteMain::doCreateTable(Statement *stmt) {
    CreateTableStatement *s = static_cast<CreateTableStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    return Re::Success;
}
