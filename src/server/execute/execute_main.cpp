#include "execute_main.h"
#include <assert.h>
RE ExecuteMain::handle()
{
    ResolveSession *rs = static_cast<ResolveSession *>(resolve_session_);
    Statement *stmt = rs->getSTMT();
    assert(stmt != nullptr);
    switch (stmt->getType())
    {
    case STMT::Select:
        doSelect(stmt);
        break;
    case STMT::CreateTable:
        doCreateTable(stmt);
        break;
    default:
        break;
    }
    return RE::SUCCESS;
}

Session *ExecuteMain::callBack()
{
    return execute_session_;
}

RE ExecuteMain::doSelect(Statement *stmt)
{
    SelectStatement *s = static_cast<SelectStatement *>(stmt);
    return RE::SUCCESS;
}

RE ExecuteMain::doCreateTable(Statement *stmt)
{
    CreateTableStatement *s = static_cast<CreateTableStatement *>(stmt);
    return RE::SUCCESS;
}
