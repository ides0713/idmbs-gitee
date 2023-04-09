#include "execute_main.h"
#include <cassert>

Re ExecuteMain::handle() {
    ResolveSession *rs = static_cast<ResolveSession *>(resolve_session_);
    Statement *stmt = rs->getStmt();
    (void) resolve_session_->getTxn();
    assert(stmt != nullptr);
    switch (stmt->getType()) {
        case StatementFlag::Select:
            doSelect(stmt);
            break;
        case StatementFlag::CreateTable:
            return doCreateTable(stmt);
        case StatementFlag::Insert:
            return doInsert(stmt);
        default:
            break;
    }
    execute_session_=new ExecuteSession(rs)
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
    if (db == nullptr) {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    return db->createTable(s->getTableName(), s->getAttrInfosNum(), s->getAttrInfos());
}

Re ExecuteMain::doInsert(Statement *stmt) {
    ResolveSession *rs = static_cast<ResolveSession *>(resolve_session_);
    InsertStatement *s = static_cast<InsertStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    if (db == nullptr) {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->getTable(std::string(s->getTableName()));
    Re r = table->insertRecord(rs->getTxn(), s->getValuesNum(), s->getValues());
    //todo:clog manager apply changes
    return Re::GenericError;
}
