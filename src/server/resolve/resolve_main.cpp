#include "resolve_main.h"
#include "../storage/storage_handler.h"
#include "../../common/common_defs.h"
#include <filesystem>
#include "../common/global_managers.h"

Re ResolveMain::handle() {
    ParseSession *ps = static_cast<ParseSession *>(parse_session_);
    Query *q = ps->getQuery();
    GlobalDataBaseManager dbm = GlobalManagers::globalDataBaseManager();
    DataBase *default_db = dbm.getDb(dbm.getProjectDefaultDatabasePath());
    if (default_db == nullptr) {
        debugPrint("ResolveMain:open default db failed,get nullptr,get default db failed\n");
        return Re::Fail;
    }
    parse_session_->setDb(default_db);
    Statement::createStatement(q, stmt_);
    if (stmt_ == nullptr) {
        debugPrint("ResolveMain:create statement failed\n");
        return Re::Fail;
    }
    stmt_->initialize(q);
    stmt_->handle(q);
    resolve_session_ = new ResolveSession(parse_session_, stmt_);
    q->destroy();
    return Re::Success;
}

Session *ResolveMain::callBack() {
    return resolve_session_;
}
