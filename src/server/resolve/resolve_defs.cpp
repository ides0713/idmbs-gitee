#include "resolve_defs.h"
#include "../../common/common_defs.h"
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"
#include "../common/re.h"
#include "../parse/parse_defs.h"
#include "../storage/database.h"
#include "../storage/table.h"
#include "../storage/txn.h"
#include "filter.h"
#include "resolve_main.h"
#include <cassert>
#include <string.h>
#include <string>
#include <unordered_map>
#include <utility>
void WildcardFields(Table *table, std::vector<Field> &fields) {
    const TableMeta &table_meta = table->GetTableMeta();
    for (int i = TableMeta::GetSysFieldsNum(); i < table_meta.GetFieldsNum(); i++)
        fields.emplace_back(table, table_meta.GetField(i));
}
void Statement::CreateStatement(Query *const query, Statement *&stmt) {
    switch (query->GetScf()) {
        case SqlCommandFlag::ScfCreateTable:
            stmt = new CreateTableStatement(query);
            break;
        case SqlCommandFlag::ScfInsert:
            stmt = new InsertStatement(query);
            break;
        case SqlCommandFlag::ScfSelect:
            stmt = new SelectStatement(query);
            break;
        case SqlCommandFlag::ScfDelete:
            stmt = new DeleteStatement(query);
            break;
        case SqlCommandFlag::ScfCreateIndex:
            stmt = new CreateIndexStatement(query);
            break;
        case SqlCommandFlag::ScfDropTable:
            stmt = new DropTableStatement(query);
            break;
        default:
            DebugPrint("Statement:unrecognized query SCF\n");
            assert(false);
    }
}
SelectStatement::SelectStatement(Query *query)
    : Statement(query->GetScf()), table_names_(nullptr), attrs_(nullptr), conditions_(nullptr), filter_(nullptr),
      attrs_num_(0), conditions_num_(0), table_names_num_(0) {
}
void SelectStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfSelect);
    auto sq = static_cast<SelectQuery *>(query);
    table_names_num_ = sq->GetRelNamesNum();
    attrs_num_ = sq->GetAttrsNum();
    conditions_num_ = sq->GetConditionsNum();
    table_names_ = new char *[table_names_num_];
    char **table_names = sq->GetRelNames();
    for (int i = 0; i < table_names_num_; i++)
        table_names_[i] = StrNew(table_names[i]);
    attrs_ = new RelAttr[attrs_num_];
    RelAttr *attrs = sq->GetAttrs();
    for (int i = 0; i < attrs_num_; i++)
        attrs_[i].Copy(attrs[i]);
    conditions_ = new Condition[conditions_num_];
    Condition *conditions = sq->GetConditions();
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].Copy(conditions[i]);
    filter_ = nullptr;
}
Re SelectStatement::Handle(Query *query, ResolveMain *resolve_main) {
    DataBase *current_db = resolve_main->GetDb();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (current_db == nullptr) {
        DebugPrint("SelectStatement:invalid argument.can not get db\n");
        gmm.SetResponse("SELECT FAILED,CAN NOT OPEN DATABASE.\n");
        return Re::InvalidArgument;
    }
    std::vector<Table *> tables_vec;
    std::unordered_map<std::string, Table *> tables_map;
    for (int i = 0; i < table_names_num_; i++) {
        std::string table_name(table_names_[i]);
        Table *table = current_db->GetTable(table_name);
        if (table == nullptr) {
            DebugPrint("SelectStatement:no such table. db=%s, table_name=%s\n", current_db->GetDbName(),
                       table_name.c_str());
            gmm.SetResponse("SELECT FAILED,NO SUCH TABLE '%s'.\n", table_name.c_str());
            return Re::SchemaTableNotExist;
        }
        tables_vec.push_back(table);
        tables_map.emplace(table_name, table);
    }
    std::vector<Field> fields_vec;
    for (int i = attrs_num_ - 1; i >= 0; i--) {
        const RelAttr &attr = attrs_[i];
        const char *rel_name = attr.rel_name, *attr_name = attr.attr_name;
        if (rel_name == nullptr and strcmp(attr_name, "*") == 0) {
            //SELECT *
            for (auto &t: tables_vec)
                WildcardFields(t, fields_vec);
        } else if (rel_name != nullptr) {
            if (strcmp(rel_name, "*") == 0) {
                //only *.* is valid format
                if (strcmp(attr_name, "*") != 0) {
                    DebugPrint("SelectStatement:invalid field name while table is *. attr=%s\n", attr_name);
                    gmm.SetResponse("SELECT FAILED,SQL SYNTAX ERROR.\n");
                    return Re::SchemaFieldMissing;
                }
                for (auto &t: tables_vec)
                    WildcardFields(t, fields_vec);
            } else {
                //select from xxx.xxx
                auto it = tables_map.find(std::string(rel_name));
                if (it == tables_map.end()) {
                    DebugPrint("SelectStatement:no such table in from list:%s\n", rel_name);
                    gmm.SetResponse("SELECT FAILED,NO SUCH TABLE '%s'.\n", rel_name);
                    return Re::SchemaFieldMissing;
                }
                Table *table = it->second;
                ///select xxx.*
                if (strcmp(attr_name, "*") == 0)
                    WildcardFields(table, fields_vec);
                else {
                    const FieldMeta *field_meta = table->GetTableMeta().GetField(attr_name);
                    if (field_meta == nullptr) {
                        DebugPrint("SelectStatement:no such field:%s in the table,invalid args\n", attr_name);
                        gmm.SetResponse("SELECT FAILED,NO SUCH FIELD '%s' IN TABLE.\n", attr_name);
                        return Re::SchemaFieldMissing;
                    }
                    fields_vec.emplace_back(table, field_meta);
                }
            }
        } else {
            if (tables_vec.size() != 1) {
                DebugPrint("SelectStatement:not clearly given attrs\n");
                gmm.SetResponse("SELECT FAILED,ATTR GIVEN NOT CLEARLY.\n");
                return Re::SchemaFieldMissing;
            }
            Table *table = tables_vec[0];
            const FieldMeta *field_meta = table->GetTableMeta().GetField(attr_name);
            if (field_meta == nullptr) {
                DebugPrint("SelectStatement:no such field:%s in the table,invalid args\n", attr_name);
                gmm.SetResponse("SELECT FAILED,NO SUCH FIELD '%s' IN TABLE.\n", attr_name);
                return Re::SchemaFieldMissing;
            }
            fields_vec.emplace_back(table, field_meta);
        }
    }
    DebugPrint("SelectStatement:got %d tables in from stmt and %d fields in query stmt\n", tables_vec.size(),
               fields_vec.size());
    Table *default_table = nullptr;
    if (tables_vec.size() == 1)
        default_table = tables_vec[0];
    Filter *filter = nullptr;
    Re r = Filter::CreateFilter(current_db, default_table, &tables_map, conditions_num_, conditions_, filter);
    if (r != Re::Success) {
        DebugPrint("SelectStatement:create filter failed\n");
        if (r == Re::SchemaFieldNotExist)
            gmm.SetResponse("SELECT FAILED,SOME FIELDS NOT FOUND IN TABLE.\n");
        return r;
    }
    filter_ = filter;
    tables_.swap(tables_vec);
    fields_.swap(fields_vec);
    return Re::Success;
}
void SelectStatement::Destroy() {
    for (int i = 0; i < table_names_num_; i++)
        delete[] table_names_[i];
    delete[] table_names_;
    for (int i = 0; i < attrs_num_; i++)
        attrs_[i].Destroy();
    delete[] attrs_;
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].Destroy();
    delete[] conditions_;
    delete filter_;
}
CreateTableStatement::CreateTableStatement(Query *query)
    : Statement(query->GetScf()), table_name_(nullptr), attr_infos_(nullptr), attr_infos_num_(0) {
}
void CreateTableStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfCreateTable);
    auto ctq = static_cast<CreateTableQuery *>(query);
    attr_infos_num_ = ctq->GetAttrNum();
    table_name_ = StrNew(ctq->GetRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = ctq->GetAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].Copy(attr_infos[i]);
}
Re CreateTableStatement::Handle(Query *query, ResolveMain *resolve_main) {
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (!IsAllFieldsValid()) {
        gmm.SetResponse("CREATE TABLE '%s' FAILED,ATTRIBUTE LEN IS INVALID\n", table_name_);
        return Re::InvalidArgument;
    }
    return Re::Success;
}
void CreateTableStatement::Destroy() {
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].Destroy();
    delete[] attr_infos_;
}
bool CreateTableStatement::IsAllFieldsValid() {
    for (int i = 0; i < attr_infos_num_; i++)
        if (!IsFieldValid(attr_infos_[i]))
            return false;
    return true;
}
bool CreateTableStatement::IsFieldValid(const AttrInfo &attr) {
    switch (attr.attr_type) {
        case AttrType::Ints: {
            if (attr.attr_len != AttrInfo::DEFAULT_INTS_LENGTH)
                return false;
        } break;
        case AttrType::Floats: {
            if (attr.attr_len != AttrInfo::DEFAULT_FLOATS_LENGTH)
                return false;
        } break;
        case AttrType::Chars: {
            if (attr.attr_len <= 0)
                return false;
        } break;
        case AttrType::Dates:
        case AttrType::Undefined:
        default:
            assert(false);
    }
    return true;
}
InsertStatement::InsertStatement(Query *query)
    : Statement(query->GetScf()), table_name_(nullptr), values_(nullptr), values_num_(0) {
}
void InsertStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfInsert);
    auto iq = static_cast<InsertQuery *>(query);
    values_num_ = iq->GetValuesNum();
    table_name_ = StrNew(iq->GetRelName());
    values_ = new Value[values_num_];
    Value *values = iq->GetValues();
    for (int i = 0; i < values_num_; i++)
        values_[i].Copy(values[i]);
}
Re InsertStatement::Handle(Query *query, ResolveMain *resolve_main) {
    Txn *txn = new Txn;
    resolve_main->SetTxn(txn);
    DataBase *current_db = resolve_main->GetDb();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (current_db == nullptr or StrBlank(table_name_) or values_num_ <= 0) {
        DebugPrint("InsertStatement:invalid argument. db=%p, table_name=%p,values_num=%d\n", current_db, table_name_,
                   values_num_);
        gmm.SetResponse("INSERT ERROR,INVALID GIVEN ARGS.\n");
        return Re::InvalidArgument;
    }
    std::string table_name_str = std::string(table_name_);
    Table *table = current_db->GetTable(table_name_str);
    if (table == nullptr) {
        DebugPrint("InsertStatement:no such table. db=%s, table_name=%s\n", current_db->GetDbName(), table_name_);
        gmm.SetResponse("INSERT ERROR,NO SUCH TABLE.\n");
        return Re::SchemaTableNotExist;
    }
    const TableMeta &table_meta = table->GetTableMeta();
    int table_sys_field_num = TableMeta::GetSysFieldsNum();
    int table_user_field_num = table_meta.GetFieldsNum() - table_sys_field_num;
    // values_num not equal to that in table_meta
    if (values_num_ != table_user_field_num) {
        DebugPrint("InsertStatement:field num not equal to values num,values_num=%d,fields_num=%d\n", values_num_,
                   table_user_field_num);
        gmm.SetResponse("INSERT ERROR,VALUES NUM INVALID.\n");
        return Re::SchemaFieldMissing;
    }
    for (int i = 0; i < values_num_; i++) {
        const FieldMeta *field_meta = table_meta.GetField(i + table_sys_field_num);
        if (values_[i].type != field_meta->GetAttrType()) {
            DebugPrint("InsertStatement:field getExprType mismatch. table=%s, field=%s, field getExprType=%d, "
                       "value_type=%d\n",
                       table_name_, field_meta->GetFieldName(), field_meta->GetAttrType(), values_[i].type);
            gmm.SetResponse("INSERT ERROR,VALUES TYPE INVALID.\n");
            return Re::SchemaFieldTypeMismatch;
        }
        // todo:advanced data check
        switch (values_[i].type) {
            default:
                break;
        }
    }
    return Re::Success;
}
void InsertStatement::Destroy() {
    delete[] table_name_;
    for (int i = 0; i < values_num_; i++)
        values_[i].Destroy();
    delete[] values_;
}
DeleteStatement::DeleteStatement(Query *query)
    : Statement(query->GetScf()), table_name_(nullptr), conditions_num_(0), conditions_(nullptr), filter_(nullptr),
      table_(nullptr) {
}
void DeleteStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfDelete);
    auto dq = static_cast<DeleteQuery *>(query);
    table_name_ = StrNew(dq->GetRelName());
    conditions_num_ = dq->GetConditionsNum();
    conditions_ = new Condition[conditions_num_];
    Condition *conditions = dq->GetConditions();
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].Copy(conditions[i]);
}
Re DeleteStatement::Handle(Query *query, ResolveMain *resolve_main) {
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (StrBlank(table_name_)) {
        DebugPrint("DeleteStatement:invalid argument.can not resolve table name\n");
        gmm.SetResponse("DELETE FAILED,CAN NOT RESOLVE TABLE NAME.\n");
        return Re::InvalidArgument;
    }
    const std::string table_name_str = std::string(table_name_);
    DataBase *current_db = resolve_main->GetDb();
    if (current_db == nullptr) {
        DebugPrint("DeleteStatement:invalid argument.can not get db\n");
        gmm.SetResponse("DELETE FAILED,CAN NOT OPEN DATABASE.\n");
        return Re::InvalidArgument;
    }
    Table *table = current_db->GetTable(table_name_str);
    if (table == nullptr) {
        DebugPrint("DeleteStatement:no such table. db=%s, table_name=%s\n", current_db->GetDbName(), table_name_);
        gmm.SetResponse("DELETE FAILED,NO SUCH TABLE '%s'.\n", table_name_);
        return Re::SchemaTableNotExist;
    }
    std::unordered_map<std::string, Table *> table_map;
    table_map.emplace(table_name_str, table);
    Filter *filter = nullptr;
    Re r = Filter::CreateFilter(current_db, table, &table_map, conditions_num_, conditions_, filter);
    if (r != Re::Success) {
        DebugPrint("DeleteStatement:failed to create filter. r=%d:%s\n", r, StrRe(r));
        if (r == Re::SchemaFieldNotExist)
            gmm.SetResponse("DELETE FAILED,SOME FIELDS NOT FOUND IN TABLE.\n");
        return r;
    }
    filter_ = filter;
    table_ = table;
    return Re::Success;
}
void DeleteStatement::Destroy() {
    delete[] table_name_;
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].Destroy();
    delete[] conditions_;
}
CreateIndexStatement::CreateIndexStatement(Query *query)
    : Statement(query->GetScf()), index_name_(nullptr), attr_(nullptr) {
}
void CreateIndexStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfCreateIndex);
    auto ciq = static_cast<CreateIndexQuery *>(query);
    index_name_ = StrNew(ciq->GetIndexName());
    attr_ = new RelAttr;
    attr_->Copy(*ciq->GetAttr());
}
Re CreateIndexStatement::Handle(Query *query, ResolveMain *resolve_main) {
    return Re::Success;
}
void CreateIndexStatement::Destroy() {
    delete[] index_name_;
    attr_->Destroy();
    delete attr_;
}
DropTableStatement::DropTableStatement(Query *query) : Statement(query->GetScf()), table_name_(nullptr) {
}
void DropTableStatement::Init(Query *query) {
    assert(this->GetScf() == SqlCommandFlag::ScfDropTable);
    auto dts = static_cast<DropTableQuery *>(query);
    table_name_ = StrNew(dts->GetRelName());
}
Re DropTableStatement::Handle(Query *query, ResolveMain *resolve_main) {
    return Re::Success;
}
void DropTableStatement::Destroy() {
    delete[] table_name_;
}
