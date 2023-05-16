#include "resolve_main.h"
#include "../../common/common_defs.h"
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"
#include "../common/re.h"
#include "../parse/parse_main.h"
#include "../storage/database.h"
#include "resolve_defs.h"
class BaseMain;
Re ResolveMain::Init(BaseMain *last_main) {
    BaseSet(*last_main);
    auto parse_main = static_cast<ParseMain *>(last_main);
    query_ = parse_main->GetQuery();
    if (query_ == nullptr)
        return Re::GenericError;
    return Re::Success;
}
Re ResolveMain::Handle() {
    GlobalDataBaseManager &dbm = GlobalManagers::GetGlobalDataBaseManager();
    DataBase *default_db = dbm.GetDb(dbm.GetProjectDefaultDatabasePath());
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (default_db == nullptr) {
        DebugPrint("ResolveMain:open default db failed,get nullptr,getFrame default db failed\n");
        gmm.SetResponse("CAN NOT OPEN CURRENT DATABASE.\n");
        return Re::GenericError;
    }
    database_ = default_db;
    Statement::CreateStatement(query_, stmt_);
    if (stmt_ == nullptr) {
        DebugPrint("ResolveMain:createFilter statement failed\n");
        gmm.SetResponse("CAN NOT RESOLVE SQL STATEMENT.\n");
        return Re::GenericError;
    }
    stmt_->Init(query_);
    Re r = stmt_->Handle(query_, this);
    if (r != Re::Success) {
        DebugPrint("ResolveMain:statement initialize and handle failed re=%d\n", r);
        return r;
    }
    return Re::Success;
}
void ResolveMain::Clear() {
    if (query_ != nullptr)
        query_ = nullptr;
    if (stmt_ != nullptr) {
        stmt_->Destroy();
        stmt_ = nullptr;
    }
}
void ResolveMain::Destroy() {
    Clear();
}