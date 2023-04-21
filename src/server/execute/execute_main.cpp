#include "execute_main.h"
#include <cassert>
#include <sstream>
#include "table_scan_operator.h"
#include "predicate_operator.h"
#include "index_scan_operator.h"
#include "project_operator.h"
#include "../resolve/resolve_main.h"
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
        if (spec->getAlias() != nullptr)
            os << spec->getAlias();
    }
    if (units_num > 0)
        os << '\n';
}

void descStrTuple(std::ostream &os, Tuple *tuple)
{
    TupleUnit unit;
    bool first_field = true;
    for (int i = 0; i < tuple->getUnitsNum(); i++)
    {
        Re r = tuple->getUnitAt(i, unit);
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

Re ExecuteMain::init(BaseMain *last_main)
{
    baseSet(*last_main);
    auto resolve_main = static_cast<ResolveMain *>(last_main);
    stmt_ = resolve_main->getStmt();
    if (stmt_ == nullptr)
        return Re::GenericError;
    return Re::Success;
}

Re ExecuteMain::handle()
{
    Re r = Re::Success;
    // (void)getTxn();
    switch (stmt_->getType())
    {
    case StatementType::Select:
        r = doSelect(stmt_);
        break;
    case StatementType::CreateTable:
        r = doCreateTable(stmt_);
        break;
    case StatementType::Insert:
        r = doInsert(stmt_);
        break;
    default:
        r = Re::GenericError;
        break;
    }
    return r;
}

void ExecuteMain::clear()
{
    if (stmt_ != nullptr)
        stmt_ = nullptr;
}

void ExecuteMain::destroy()
{
    clear();
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
    PredicateOperator *pred_oper = new PredicateOperator(s->getFilter());
    pred_oper->addOper(scan_oper);
    ProjectOperator *project_oper = new ProjectOperator();
    project_oper->addOper(pred_oper);
    for (const Field &field : *s->getFields())
        project_oper->addProjection(field.getTable(), field.getFieldMeta());
    Re r = project_oper->init();
    if (r != Re::Success)
    {
        debugPrint("ExecuteMain:failed to init operator\n");
        return r;
    }
    std::stringstream sstream;
    printTupleHeader(sstream, *project_oper);
    while ((r = project_oper->handle()) == Re::Success)
    {
        Tuple *tuple = project_oper->getCurrentTuple();
        if (tuple == nullptr)
        {
            r = Re::Internal;
            debugPrint("ExecuteMain:failed to get current record. re=%s\n", strRe(r));
            break;
        }
        descStrTuple(sstream, tuple);
        sstream << std::endl;
    }
    if (r != Re::RecordEof)
    {
        debugPrint("ExecuteMain:something wrong while iterate operator. re=%s\n", strRe(r));
        project_oper->destroy();
    }
    else
        r = project_oper->destroy();
    const std::string &str = sstream.str();
    GlobalMainManager &gmm = GlobalManagers::globalMainManager();
    gmm.setResponse(str);
    return r;
}

Re ExecuteMain::doCreateTable(Statement *stmt)
{
    auto s = static_cast<CreateTableStatement *>(stmt);
    DataBase *db = getDb();
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
    DataBase *db = getDb();
    Txn *txn = getTxn();
    if (db == nullptr)
    {
        debugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->getTable(std::string(s->getTableName()));
    Re r = table->insertRecord(txn, s->getValuesNum(), s->getValues());
    if (r != Re::Success)
        return r;
    CLogManager *clog_manager = db->getCLogManager();
    if (!getTMO())
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