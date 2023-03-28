#include "resolve_main.h"
#include "../storage/storage_handler.h"
#include <cstdio>
#include "../../common/common_defs.h"

Re ResolveMain::handle() {
    ParseSession *ps = static_cast<ParseSession *>(parse_session_);
    Query *q = ps->getQuery();
    //todo:db->getdb check db
//    DataBase* current_db=
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
