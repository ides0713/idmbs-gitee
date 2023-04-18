#include "execute_main.h"
#include <cassert>
#include <sstream>
#include "table_scan_operator.h"
#include "predicate_operator.h"
#include "index_scan_operator.h"
#include "project_operator.h"
IndexScanOperator *createIndexScanOperator(Filter *filter)
{
    // TODO: implement index
    return nullptr;
}
void printTupleHeader(std::ostream &os, const ProjectOperator &oper)
{
    const int units_num = oper.getTupleUnitsNum();
    const TupleUnitSpec *spec = nullptr;
    for (int i = 0; i < units_num; i++)
    {
        oper.getTupleUnitAt(i, spec);
        if (i != 0)
            os << " | ";
        // if(spec->getAlias()==nullptr)
        // printf("oops something wrong\n");
        // printf("alias is %s\n",spec->getAlias());
        if (spec->getAlias() != nullptr)
            os << spec->getAlias();
    }
    if (units_num > 0)
        os << '\n';
}

void descStrTuple(std::ostream &os, const Tuple &tuple)
{
    TupleUnit unit;
    bool first_field = true;
    for (int i = 0; i < tuple.getUnitsNum(); i++)
    {
        Re r = tuple.getUnitAt(i, unit);
        if (r != Re::Success)
        {
            debugPrint("descStrTuple:failed to fetch field of cell. index=%d, re=%s", i, strRe(r));
            break;
        }
        if (!first_field)
            os << " | ";
        else
            first_field = false;
        unit.toString(os);
    }
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
    if (s->getTables()->size() != 1)
    {
        debugPrint("ExecuteMain:select more than 1 tables is not supported\n");
        return Re::NotImplement;
    }
    Table *const table = (*s->getTables())[0];
    Operator *scan_oper = createIndexScanOperator(s->getFilter());
    if (scan_oper == nullptr)
        scan_oper = new TableScanOperator(table);
    PredicateOperator pred_oper(s->getFilter());
    pred_oper.addOper(scan_oper);
    ProjectOperator project_oper;
    project_oper.addOper(&pred_oper);
    for (const Field &field : *s->getFields())
        project_oper.addProjection(field.getTable(), field.getFieldMeta());
    Re r = project_oper.init();
    if (r != Re::Success)
    {
        debugPrint("ExecuteMain:failed to init operator\n");
        return r;
    }
    std::stringstream sstream;
    printTupleHeader(sstream, project_oper);
    //todo cannot handle project operator because record iterator no work
    //insert or record iterator?
    while ((r = project_oper.handle()) == Re::Success)
    {
        printf("wuhu we enter the loop finally\n");
        Tuple *tuple = project_oper.getCurrentTuple();
        if (tuple = nullptr)
        {
            r = Re::Internal;
            debugPrint("ExecuteMain:failed to get current record. re=%s\n", strRe(r));
            break;
        }
        descStrTuple(sstream, *tuple);
        sstream << std::endl;
    }
    if (r != Re::RecordEof)
    {
        debugPrint("ExecuteMain:something wrong while iterate operator. re=%s\n", strRe(r));
        project_oper.destroy();
    }
    else
        r = project_oper.destroy();
    const std::string& str = sstream.str();
    printf("%s",str.c_str());
    return r;
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
    printf("%s", resolve_session_->getResponse().c_str());
}
