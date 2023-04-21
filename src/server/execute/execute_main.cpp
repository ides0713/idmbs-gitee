#include "execute_main.h"
#include "../resolve/resolve_main.h"
#include "../storage/txn.h"
#include "index_scan_operator.h"
#include "predicate_operator.h"
#include "project_operator.h"
#include "table_scan_operator.h"
#include <cassert>
#include <sstream>

IndexScanOperator *CreateIndexScanOperator(Filter *filter) {
    // TODO: implement index
    return nullptr;
}
void PrintTupleHeader(std::ostream &os, const ProjectOperator &oper) {
    const int units_num = oper.GetTupleUnitsNum();
    const TupleUnitSpec *spec = nullptr;
    for (int i = 0; i < units_num; i++) {
        oper.GetTupleUnitAt(i, spec);
        if (i != 0)
            os << " | ";
        if (spec->GetAlias() != nullptr)
            os << spec->GetAlias();
    }
    if (units_num > 0)
        os << '\n';
}

void DescStrTuple(std::ostream &os, Tuple *tuple) {
    TupleUnit unit;
    bool first_field = true;
    for (int i = 0; i < tuple->GetUnitsNum(); i++) {
        Re r = tuple->GetUnitAt(i, unit);
        if (r != Re::Success) {
            DebugPrint("descStrTuple:failed to fetch field of cell. index=%d, re=%s", i, StrRe(r));
            break;
        }
        if (!first_field)
            os << " | ";
        else
            first_field = false;
        unit.ToString(os);
    }
}

Re ExecuteMain::Init(BaseMain *last_main) {
    BaseSet(*last_main);
    auto resolve_main = static_cast<ResolveMain *>(last_main);
    stmt_ = resolve_main->GetStmt();
    if (stmt_ == nullptr)
        return Re::GenericError;
    return Re::Success;
}

Re ExecuteMain::Handle() {
    Re r = Re::Success;
    // (void)getTxn();
    switch (stmt_->GetType()) {
        case StatementType::Select:
            r = DoSelect(stmt_);
            break;
        case StatementType::CreateTable:
            r = DoCreateTable(stmt_);
            break;
        case StatementType::Insert:
            r = DoInsert(stmt_);
            break;
        default:
            r = Re::GenericError;
            break;
    }
    return r;
}

void ExecuteMain::Clear() {
    if (stmt_ != nullptr)
        stmt_ = nullptr;
}

void ExecuteMain::Destroy() {
    Clear();
}

Re ExecuteMain::DoSelect(Statement *stmt) {
    auto s = static_cast<SelectStatement *>(stmt);
    if (s->GetTables()->size() != 1) {
        DebugPrint("ExecuteMain:select more than 1 tables is not supported\n");
        return Re::NotImplement;
    }
    Table *const table = (*s->GetTables())[0];
    Operator *scan_oper = CreateIndexScanOperator(s->GetFilter());
    if (scan_oper == nullptr)
        scan_oper = new TableScanOperator(table);
    PredicateOperator *pred_oper = new PredicateOperator(s->GetFilter());
    pred_oper->AddOper(scan_oper);
    ProjectOperator *project_oper = new ProjectOperator();
    project_oper->AddOper(pred_oper);
    for (const Field &field: *s->GetFields())
        project_oper->AddProjection(field.GetTable(), field.GetFieldMeta());
    Re r = project_oper->Init();
    if (r != Re::Success) {
        DebugPrint("ExecuteMain:failed to init operator\n");
        return r;
    }
    std::stringstream sstream;
    PrintTupleHeader(sstream, *project_oper);
    while ((r = project_oper->Handle()) == Re::Success) {
        Tuple *tuple = project_oper->GetCurrentTuple();
        if (tuple == nullptr) {
            r = Re::Internal;
            DebugPrint("ExecuteMain:failed to get current record. re=%s\n", StrRe(r));
            break;
        }
        DescStrTuple(sstream, tuple);
        sstream << std::endl;
    }
    if (r != Re::RecordEof) {
        DebugPrint("ExecuteMain:something wrong while iterate operator. re=%s\n", StrRe(r));
        project_oper->Destroy();
    } else
        r = project_oper->Destroy();
    const std::string &str = sstream.str();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    gmm.SetResponse(str);
    return r;
}

Re ExecuteMain::DoCreateTable(Statement *stmt) {
    auto s = static_cast<CreateTableStatement *>(stmt);
    DataBase *db = GetDb();
    if (db == nullptr) {
        DebugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    return db->CreateTable(s->GetTableName(), s->GetAttrInfosNum(), s->GetAttrInfos());
}

Re ExecuteMain::DoInsert(Statement *stmt) {
    //    auto rs = static_cast<ResolveSession *>(resolve_session_);
    auto s = static_cast<InsertStatement *>(stmt);
    DataBase *db = GetDb();
    Txn *txn = GetTxn();
    if (db == nullptr) {
        DebugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->GetTable(std::string(s->GetTableName()));
    Re r = table->InsertRecord(txn, s->GetValuesNum(), s->GetValues());
    if (r != Re::Success)
        return r;
    CLogManager *clog_manager = db->GetCLogManager();
    if (!GetTmo()) {
        CLogRecord *clog_record = nullptr;
        r = clog_manager->MakeRecord(CLogType::RedoMiniTxnCommit, txn->GetTxnId(), clog_record);
        if (r != Re::Success or clog_record == nullptr)
            return r;
        r = clog_manager->AppendRecord(clog_record);
        if (r != Re::Success)
            return r;
        txn->NextCurrentId();
    }
    return Re::Success;
}