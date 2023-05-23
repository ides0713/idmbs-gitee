#include "execute_main.h"
#include "../../common/common_defs.h"
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"
#include "../common/re.h"
#include "../resolve/expression.h"
#include "../resolve/filter.h"
#include "../resolve/resolve_defs.h"
#include "../resolve/resolve_main.h"
#include "../resolve/tuple.h"
#include "../storage/clog_manager.h"
#include "../storage/database.h"
#include "../storage/field.h"
#include "../storage/table.h"
#include "../storage/txn.h"
#include "delete_operator.h"
#include "index_scan_operator.h"
#include "predicate_operator.h"
#include "project_operator.h"
#include "table_scan_operator.h"
#include <sstream>
#include <string>
#include <vector>
IndexScanOperator *CreateIndexScanOperator(Filter *filter) {
    const std::vector<FilterUnit *> &filter_units = filter->GetFilterUnits();
    if (filter_units.empty())
        return nullptr;
    // 在所有过滤条件中，找到字段与值做比较的条件，然后判断字段是否可以使用索引
    // 如果是多列索引，这里的处理需要更复杂。
    // 这里的查找规则是比较简单的，就是尽量找到使用相等比较的索引
    // 如果没有就找范围比较的，但是直接排除不等比较的索引查询.
    const FilterUnit *better_filter = nullptr;
    for (const FilterUnit *filter_unit: filter_units) {
        if (filter_unit->GetComp() == CompOp::NotEqual)
            continue;
        Expression *left = filter_unit->GetLeft(), *right = filter_unit->GetRight();
        if (left->GetExprType() == ExprType::Field and right->GetExprType() == ExprType::Value) {
        } else if (left->GetExprType() == ExprType::Value and right->GetExprType() == ExprType::Field) {
            std::swap(left, right);
        } else
            continue;
        FieldExpression &left_field_expr = *static_cast<FieldExpression *>(left);
        const Field &field = left_field_expr.GetField();
        const Table *table = field.GetTable();
        Index *index = table->GetIndexByField(field.GetFieldName());
        if (index != nullptr) {
            if (better_filter == nullptr)
                better_filter = filter_unit;
            else if (filter_unit->GetComp() == CompOp::EqualTo) {
                better_filter = filter_unit;
                break;
            }
        }
    }
    if (better_filter == nullptr)
        return nullptr;
    Expression *left = better_filter->GetLeft(), *right = better_filter->GetRight();
    CompOp comp = better_filter->GetComp();
    if (left->GetExprType() == ExprType::Value and right->GetExprType() == ExprType::Field) {
        std::swap(left, right);
        switch (comp) {
            case CompOp::EqualTo:
            case CompOp::NotEqual:
                break;
            case CompOp::LessEqual: {
                comp = CompOp::GreatThan;
            } break;
            case CompOp::LessThan: {
                comp = CompOp::GreatEqual;
            } break;
            case CompOp::GreatEqual: {
                comp = CompOp::LessThan;
            } break;
            case CompOp::GreatThan: {
                comp = CompOp::LessEqual;
            } break;
            default: {
                DebugPrint("execute_main:should not happen\n");
            } break;
        }
    }
    FieldExpression &left_field_expr = *static_cast<FieldExpression *>(left);
    const Field &field = left_field_expr.GetField();
    const Table *table = field.GetTable();
    Index *index = table->GetIndexByField(field.GetFieldName());
    assert(index != nullptr);
    ValueExpression &right_value_expr = *static_cast<ValueExpression *>(right);
    TupleUnit value;
    right_value_expr.GetTupleUnit(value);
    const TupleUnit *left_unit = nullptr, *right_unit = nullptr;
    bool left_inclusive = false, right_inclusive = false;
    switch (comp) {
        case CompOp::EqualTo: {
            left_unit = &value, right_unit = &value;
            left_inclusive = true, right_inclusive = true;
        } break;
        case CompOp::LessEqual: {
            left_unit = nullptr, right_unit = &value;
            left_inclusive = false, right_inclusive;
        } break;
        case CompOp::LessThan: {
            left_unit = nullptr, right_unit = &value;
            left_inclusive = false, right_inclusive = false;
        } break;
        case CompOp::GreatEqual: {
            left_unit = &value, right_unit = nullptr;
            left_inclusive = true, right_inclusive = false;
        } break;
        case CompOp::GreatThan: {
            left_unit = &value, right_unit = nullptr;
            left_inclusive = false, right_inclusive = false;
        } break;
        default: {
            DebugPrint("execute_main:should not happen. comp=%d:%s\n", comp, StrCompOp(comp));
        } break;
    }
    IndexScanOperator *oper =
            new IndexScanOperator(table, index, left_unit, left_inclusive, right_unit, right_inclusive);
    DebugPrint("execute_main:use index for scan: %s in table %s\n", index->GetIndexMeta().GetIndexName(),
               table->GetTableName());
    return oper;
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
    const int units_num = tuple->GetUnitsNum();
    for (int i = 0; i < units_num; i++) {
        Re r = tuple->GetUnitAt(i, unit);
        if (r != Re::Success) {
            DebugPrint("descStrTuple:failed to fetch field of cell. index=%d, r=%s", i, StrRe(r));
            break;
        }
        if (i != 0)
            os << " | ";
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
        case StatementType::Delete:
            r = DoDelete(stmt_);
            break;
        case StatementType::CreateIndex:
            r = DoCreateIndex(stmt_);
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
            DebugPrint("ExecuteMain:failed to get current record. r=%s\n", StrRe(r));
            break;
        }
        DescStrTuple(sstream, tuple);
        sstream << std::endl;
    }
    if (r != Re::RecordEof) {
        DebugPrint("ExecuteMain:something wrong while iterate operator. r=%s\n", StrRe(r));
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
    Re r = db->CreateTable(s->GetTableName(), s->GetAttrInfosNum(), s->GetAttrInfos());
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (r != Re::Success) {
        DebugPrint("ExecuteMain:create table failed,r=%d:%s\n", r, StrRe(r));
        gmm.SetResponse("CREATE TABLE '%s' FAILED.\n", s->GetTableName());
        return r;
    }
    gmm.SetResponse("CREATE TABLE SUCCEEDED.\n");
    return Re::Success;
}
Re ExecuteMain::DoInsert(Statement *stmt) {
    auto s = static_cast<InsertStatement *>(stmt);
    DataBase *db = GetDb();
    Txn *txn = GetTxn();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (db == nullptr) {
        DebugPrint("ExecuteMain:getDb failed,no db was set\n");
        return Re::SchemaDbNotExist;
    }
    Table *table = db->GetTable(std::string(s->GetTableName()));
    Re r = table->InsertRecord(txn, s->GetValuesNum(), s->GetValues());
    if (r != Re::Success) {
        gmm.SetResponse("INSERT FAILED,INSERT RECORD TO TABLE '%s' FAILED.\n", table->GetTableName());
        return r;
    }
    CLogManager *clog_manager = db->GetCLogManager();
    if (!GetTmo()) {
        CLogRecord *clog_record = nullptr;
        r = clog_manager->MakeRecord(CLogType::RedoMiniTxnCommit, txn->GetTxnId(), clog_record);
        if (r != Re::Success or clog_record == nullptr)
            return r;
        r = clog_manager->AppendRecord(clog_record);
        if (r != Re::Success) {
            gmm.SetResponse("INSERT FAILED,CAN NOT APPEND LOG.\n");
            return r;
        }
        txn->NextCurrentId();
        gmm.SetResponse("INSERT SUCCEEDED.\n");
    } else
        gmm.SetResponse("INSERT SUCCEEDED.\n");
    return Re::Success;
}
Re ExecuteMain::DoDelete(Statement *stmt) {
    DataBase *current_database = GetDb();
    Txn *txn = GetTxn();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    CLogManager *clog_manager = current_database->GetCLogManager();
    if (stmt == nullptr) {
        DebugPrint("ExecuteMain:cannot find delete statement\n");
        gmm.SetResponse("DELETE FAILED.\n");
        return Re::GenericError;
    }
    auto ds = static_cast<DeleteStatement *>(stmt);
    TableScanOperator *scan_oper = new TableScanOperator(ds->GetTable());
    PredicateOperator *pred_oper = new PredicateOperator(ds->GetFilter());
    pred_oper->AddOper(scan_oper);
    DeleteOperator *del_oper = new DeleteOperator(ds, txn);
    del_oper->AddOper(pred_oper);
    Re r = del_oper->Init();
    if (r != Re::Success) {
        DebugPrint("ExecuteMain:init operators failed\n");
        gmm.SetResponse("DELETE FAILED,INIT OPERATOR FAILED.\n");
        return r;
    }
    r = del_oper->Handle();
    if (r != Re::Success) {
        DebugPrint("ExecuteMain:handle operators failed\n");
        gmm.SetResponse("DELETE FAILED,HANDLE OPERATOR FAILED.\n");
        return r;
    }
    if (!GetTmo()) {
        CLogRecord *clog_record = nullptr;
        r = clog_manager->MakeRecord(CLogType::RedoMiniTxnCommit, txn->GetTxnId(), clog_record);
        if (r != Re::Success or clog_record == nullptr) {
            DebugPrint("ExecuteMain:make clog record failed r:%d,%s\n", r, StrRe(r));
            gmm.ClearResponse();
            gmm.SetResponse("DELETE FAILED,CAN NOT MAKE LOG.\n");
            return r;
        }
        r = clog_manager->AppendRecord(clog_record);
        if (r != Re::Success) {
            DebugPrint("ExecuteMain:append clog record failed r:%d,%s\n", r, StrRe(r));
            gmm.ClearResponse();
            gmm.SetResponse("DELETE FAILED,CAN NOT APPEND LOG.\n");
            return r;
        }
        txn->NextCurrentId();
    }
    return Re::Success;
}
Re ExecuteMain::DoCreateIndex(Statement *stmt) {
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    DataBase *current_database = GetDb();
    auto cis = static_cast<CreateIndexStatement *>(stmt);
    RelAttr *attr = cis->GetAttr();
    std::string table_name = std::string(attr->rel_name);
    Table *table = current_database->GetTable(table_name);
    if (table == nullptr) {
        DebugPrint("ExecuteMain:get table:%s failed,no such table\n", table_name.c_str());
        gmm.SetResponse("CREATE INDEX FAILED,NO SUCH TABLE '%s'.\n", table_name.c_str());
        return Re::SchemaTableNotExist;
    }
    Re r = table->CreateIndex(nullptr, cis->GetIndexName(), attr->attr_name);
    if (r != Re::Success) {
        DebugPrint("ExecuteMain:create index failed r=%d,%s\n", r, StrRe(r));
        gmm.SetResponse("CREATE INDEX FAILED,CREATE INDEX '%s' ON '%s.%s' FAILED.\n", cis->GetIndexName(),
                        attr->rel_name, attr->attr_name);
        return r;
    }
    gmm.SetResponse("CREATE INDEX SUCCEEDED.\n");
    return Re::Success;
}
Re ExecuteMain::DoDropTable(Statement *stmt) {
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    DataBase *current_database = GetDb();
    auto dts = static_cast<DropTableStatement *>(stmt);
    std::string table_name = std::string(dts->GetTableName());
    Table *table = current_database->GetTable(table_name);
    if(table==nullptr){
        DebugPrint("ExecuteMain:drop table %s failed,no such table\n");
        gmm.SetResponse("DROP TABLE FAILED,NO SUCH TABLE '%s'.\n",table_name.c_str());
        return Re::SchemaTableNotExist;
    }
    Re r=current_database->DropTable(table_name.c_str());
    if(r!=Re::Success){
        DebugPrint("ExecuteMain:drop table %s failed,r=%d:%s\n",table_name.c_str(),r,StrRe(r));
        gmm.SetResponse("DROP TABLE FAILED.\n");
        return r;
    }
    gmm.SetResponse("DROP TABLE SUCCEEDED.\n");
    return Re::Success;
    //   SessionEvent *session_event = sql_event->session_event();
    //   Db *db = session_event->session()->get_current_db();
    //   const DropTable &drop_table = sql_event->query()->sstr.drop_table;
    //   Table *table = db->find_table(drop_table.relation_name);
    //   if (table == nullptr) {
    //     session_event->set_response("FAILURE\n");
    //     return RC::SCHEMA_TABLE_NOT_EXIST;
    //   }
    //   RC rc = db->drop_table(drop_table.relation_name);
    //   sql_event->session_event()->set_response(rc == RC::SUCCESS ? "SUCCESS\n" : "FAILURE\n");
    //   return rc;
}
