#include "execute_main.h"
#include <cassert>
#include "table_scan_operator.h"
#include "predicate_operator.h"
#include "index_scan_operator.h"

IndexScanOperator *createIndexScanOperator(Filter *filter)
{
    // TODO: implement index
    return nullptr;
}
Re ExecuteMain::handle()
{
    auto rs = static_cast<ResolveSession *>(resolve_session_);
    Re r = Re::Success;
    Statement *stmt = rs->getStmt();
    (void)resolve_session_->getTxn();
    assert(stmt != nullptr);
    switch (stmt->getType())
    {
    case StatementType::Select:
        r = doSelect(stmt);
        break;
    case StatementType::CreateTable:
        r = doCreateTable(stmt);
        break;
    case StatementType::Insert:
        r = doInsert(stmt);
        break;
    default:
        r = Re::GenericError;
        break;
    }
    execute_session_ = new ExecuteSession(rs);
    return r;
}

Session *ExecuteMain::callBack()
{
    return execute_session_;
}

Re ExecuteMain::doSelect(Statement *stmt)
{
    auto s = static_cast<SelectStatement *>(stmt);
    resolve_session_->setResponse("do select not implemented yet");
    if (s->getTables()->size() != 1)
    {
        debugPrint("ExecuteMain:select more than 1 tables is not supported\n");
        return Re::NotImplement;
    }
    Table *const table = (*s->getTables())[0];
    Operator *scan_oper = createIndexScanOperator(s->getFilter());
    if (scan_oper == nullptr)
        scan_oper = new TableScanOperator(table);

    return Re::GenericError;
}

Re ExecuteMain::doCreateTable(Statement *stmt)
{
    auto s = static_cast<CreateTableStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    if (db == nullptr)
    {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    return db->createTable(s->getTableName(), s->getAttrInfosNum(), s->getAttrInfos());
}

Re ExecuteMain::doInsert(Statement *stmt)
{
    //    auto rs = static_cast<ResolveSession *>(resolve_session_);
    auto s = static_cast<InsertStatement *>(stmt);
    DataBase *db = resolve_session_->getDb();
    Txn *txn = resolve_session_->getTxn();
    if (db == nullptr)
    {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->getTable(std::string(s->getTableName()));
    Re r = table->insertRecord(txn, s->getValuesNum(), s->getValues());
    // todo:clog manager apply changes
    if (r != Re::Success)
        return r;
    CLogManager *clog_manager = db->getCLogManager();
    if (!resolve_session_->getTmo())
    {
        CLogRecord *clog_record = nullptr;
        r = clog_manager->makeRecord(CLogType::RedoMiniTxnCommit, txn->getTxnId(), clog_record);
        if (r != Re::Success or clog_record == nullptr)
            return r;
        r = clog_manager->appendRecord(clog_record);
        if (r != Re::Success)
            return r;
        txn->nextCurrentId();
    }
    return Re::Success;
}

void ExecuteMain::response()
{
    printf("%s\n", resolve_session_->getResponse());
}
