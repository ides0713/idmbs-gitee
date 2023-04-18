#include "resolve_main.h"
#include "../storage/storage_handler.h"

Re ResolveMain::handle()
{
    auto *ps = static_cast<ParseSession *>(parse_session_);
    Query *q = ps->getQuery();
    GlobalDataBaseManager dbm = GlobalManagers::globalDataBaseManager();
    DataBase *default_db = dbm.getDb(dbm.getProjectDefaultDatabasePath());
    if (default_db == nullptr)
    {
        debugPrint("ResolveMain:open default db failed,get nullptr,getFrame default db failed\n");
        parse_session_->setResponse("CAN NOT OPEN CURRENT DATABASE.\n");
        return Re::GenericError;
    }
    parse_session_->setDb(default_db);
    Statement::createStatement(q, stmt_);
    if (stmt_ == nullptr)
    {
        debugPrint("ResolveMain:createFilter statement failed\n");
        parse_session_->setResponse("CAN NOT RESOLVE SQL STATEMENT.\n");
        return Re::GenericError;
    }
    stmt_->init(q);
    Re r = stmt_->handle(q, parse_session_);
    if (r != Re::Success)
    {
        debugPrint("ResolveMain:statement initialize and handle failed re=%d\n", r);
        return r;
    }
    resolve_session_ = new ResolveSession(parse_session_, stmt_);
    q->destroy();
    return Re::Success;
}

Session *ResolveMain::callBack()
{
    return resolve_session_;
}

void ResolveMain::response()
{
    printf("%s", parse_session_->getResponse().c_str());
}

void ResolveMain::stmtSucceed()
{
    switch (stmt_->getType())
    {
    case StatementType::CreateTable:
        printf("CREATE TABLE %s SUCCEEDED\n", static_cast<CreateTableStatement *>(stmt_)->getTableName());
        break;
    case StatementType::Insert:
        printf("INSERT RECORD SUCCEEDED\n");
        break;
    case StatementType::Select:
        printf("SELECT FINISH\n");
        break;
    default:
        printf("SQL SUCCEEDED BUT NO MSG RETURNED\n");
        break;
    }
}

void ResolveMain::stmtDestroyed()
{
    stmt_->destroy();
}
