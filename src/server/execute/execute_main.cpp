#include "execute_main.h"
#include <cassert>

Re ExecuteMain::handle() {
    auto rs = static_cast<ResolveSession *>(resolve_session_);
    Re r = Re::Success;
    Statement *stmt = rs->getStmt();
    (void) resolve_session_->getTxn();
    assert(stmt != nullptr);
    switch (stmt->getType()) {
        case StatementFlag::Select:
            r = doSelect(stmt);
            break;
        case StatementFlag::CreateTable:
            r = doCreateTable(stmt);
            break;
        case StatementFlag::Insert:
            r = doInsert(stmt);
            break;
        default:
            r = Re::GenericError;
            break;
    }
    execute_session_ = new ExecuteSession(rs);
    return r;
}

Session *ExecuteMain::callBack() {
    return execute_session_;
}

Re ExecuteMain::doSelect(Statement *stmt) {
    auto s = static_cast<SelectStatement *>(stmt);
    return Re::Success;
}

Re ExecuteMain::doCreateTable(Statement *stmt) {
    auto s = static_cast<CreateTableStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    if (db == nullptr) {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    return db->createTable(s->getTableName(), s->getAttrInfosNum(), s->getAttrInfos());
}

Re ExecuteMain::doInsert(Statement *stmt) {
//    auto rs = static_cast<ResolveSession *>(resolve_session_);
    auto s = static_cast<InsertStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    Txn *txn = resolve_session_->getTxn();
    if (db == nullptr) {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->getTable(std::string(s->getTableName()));
    Re r = table->insertRecord(txn, s->getValuesNum(), s->getValues());
    //todo:clog manager apply changes
    if (r != Re::Success)
        return r;
    CLogManager *clog_manager = db->getCLogManager();
    if (!resolve_session_->getTmo()) {
        CLogRecord *clog_record = nullptr;
        r = clog_manager->makeRecord(CLogType::RedoMiniTxnCommit, txn->getCurrentTxnId(), clog_record);
        if (r != Re::Success or clog_record == nullptr)
            return r;
        r = clog_manager->appendRecord(clog_record);
        if (r != Re::Success)
            return r;
        txn->nextCurrentId();
    }
    return Re::Success;
}
