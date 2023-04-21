#include "resolve_main.h"
#include "../storage/storage_handler.h"
#include "../parse/parse_main.h"

Re ResolveMain::init(BaseMain *last_main)
{
    baseSet(*last_main);
    auto parse_main = static_cast<ParseMain *>(last_main);
    query_ = parse_main->getQuery();
    if (query_ == nullptr)
        return Re::GenericError;
    return Re::Success;
}

Re ResolveMain::handle()
{
    GlobalDataBaseManager &dbm = GlobalManagers::globalDataBaseManager();
    DataBase *default_db = dbm.getDb(dbm.getProjectDefaultDatabasePath());
    GlobalMainManager &gmm = GlobalManagers::globalMainManager();
    if (default_db == nullptr)
    {
        debugPrint("ResolveMain:open default db failed,get nullptr,getFrame default db failed\n");
        gmm.setResponse("CAN NOT OPEN CURRENT DATABASE.\n");
        return Re::GenericError;
    }
    database_ = default_db;
    Statement::createStatement(query_, stmt_);
    if (stmt_ == nullptr)
    {
        debugPrint("ResolveMain:createFilter statement failed\n");
        gmm.setResponse("CAN NOT RESOLVE SQL STATEMENT.\n");
        return Re::GenericError;
    }
    stmt_->init(query_);
    Re r = stmt_->handle(query_, this);
    if (r != Re::Success)
    {
        debugPrint("ResolveMain:statement initialize and handle failed re=%d\n", r);
        return r;
    }
    return Re::Success;
}

void ResolveMain::clear()
{
    if (query_ != nullptr)
        query_ = nullptr;
    if (stmt_ != nullptr)
    {
        stmt_->destroy();
        stmt_ = nullptr;
    }
}

void ResolveMain::destroy()
{
    clear();
}