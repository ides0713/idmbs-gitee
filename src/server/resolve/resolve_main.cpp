#include "resolve_main.h"
#include "../storage/storage_handler.h"
#include <filesystem>

Re ResolveMain::handle() {
    auto *ps = static_cast<ParseSession *>(parse_session_);
    Query *q = ps->getQuery();
    GlobalDataBaseManager dbm = GlobalManagers::globalDataBaseManager();
    DataBase *default_db = dbm.getDb(dbm.getProjectDefaultDatabasePath());
    if (default_db == nullptr) {
        debugPrint("ResolveMain:open default db failed,get nullptr,getFrame default db failed\n");
        return Re::GenericError;
    }
    parse_session_->setDb(default_db);
    Statement::createStatement(q, nullptr, stmt_);
    if (stmt_ == nullptr) {
        debugPrint("ResolveMain:create statement failed\n");
        return Re::GenericError;
    }
    stmt_->init(q);
    Re r = stmt_->handle(q, parse_session_);
    if (r != Re::Success) {
        debugPrint("ResolveMain:statement initialize and handle failed re=%d\n", r);
        return r;
    }
    resolve_session_ = new ResolveSession(parse_session_, stmt_);
    q->destroy();
    return Re::Success;
}

Session *ResolveMain::callBack() {
    return resolve_session_;
}
