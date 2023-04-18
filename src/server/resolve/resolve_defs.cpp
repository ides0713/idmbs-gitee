#include "resolve_defs.h"
#include <cassert>
#include "../common/session.h"
#include "filter.h"

void wildcardFields(Table *table, std::vector<Field> &fields)
{
    const TableMeta &table_meta = table->getTableMeta();
    const int field_num = table_meta.getFieldsNum();
    for (int i = TableMeta::getSysFieldsNum(); i < field_num; i++)
        fields.emplace_back(table, table_meta.getField(i));
}

void Statement::createStatement(Query *const query, Statement *&stmt)
{
    switch (query->getScf())
    {
    case ScfCreateTable:
        stmt = new CreateTableStatement(query);
        break;
    case ScfInsert:
        stmt = new InsertStatement(query);
        break;
    case ScfSelect:
        stmt = new SelectStatement(query);
        break;
    default:
        debugPrint("Statement:unrecognized query SCF\n");
        break;
    }
}

SelectStatement::SelectStatement(Query *query) : Statement(query->getScf())
{
    table_names_ = nullptr, attrs_ = nullptr, conditions_ = nullptr, filter_ = nullptr;
    attrs_num_ = 0, conditions_num_ = 0, table_names_num_ = 0;
}

void SelectStatement::init(Query *query)
{
    auto sq = static_cast<SelectQuery *>(query);
    assert(this->getScf() == ScfSelect);
    table_names_num_ = sq->getRelNamesNum();
    attrs_num_ = sq->getAttrsNum();
    conditions_num_ = sq->getConditionsNum();
    table_names_ = new char *[table_names_num_];
    char **table_names = sq->getRelNames();
    for (int i = 0; i < table_names_num_; i++)
        table_names_[i] = strNew(table_names[i]);
    attrs_ = new RelAttr[attrs_num_];
    RelAttr *attrs = sq->getAttrs();
    for (int i = 0; i < attrs_num_; i++)
        attrs_[i].copy(attrs[i]);
    conditions_ = new Condition[conditions_num_];
    Condition *conditions = sq->getConditions();
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].copy(conditions[i]);
    assert(this->getScf() == ScfSelect);
    filter_ = nullptr;
}

Re SelectStatement::handle(Query *query, Session *parse_session)
{
    auto ps = static_cast<ParseSession *>(parse_session);
    DataBase *current_db = ps->getDb();
    if (current_db == nullptr)
    {
        debugPrint("SelectStatement:invalid argument.can not get db\n");
        parse_session->setResponse("SELECT ERROR,CAN NOT OPEN DATABASE");
        return Re::InvalidArgument;
    }
    std::vector<Table *> tables_vec;
    std::unordered_map<std::string, Table *> tables_map;
    for (int i = 0; i < table_names_num_; i++)
    {
        std::string table_name(table_names_[i]);
        Table *table = current_db->getTable(table_name);
        if (table == nullptr)
        {
            debugPrint("SelectStatement:no such table. db=%s, table_name=%s\n",
                       current_db->getDbName().c_str(), table_name.c_str());
            parse_session->setResponse("SELECT ERROR,NO SUCH TABLE");
            return Re::SchemaTableNotExist;
        }
        tables_vec.push_back(table);
        tables_map.emplace(table_name, table);
    }
    std::vector<Field> fields_vec;
    for (int i = 0; i < attrs_num_; i++)
    {
        const RelAttr &attr = attrs_[i];
        const char *rel_name = attr.rel_name;
        const char *attr_name = attr.attr_name;
        if (rel_name == nullptr and strcmp(attr_name, "*") == 0)
            for (auto &t : tables_vec)
                wildcardFields(t, fields_vec);
        else if (rel_name != nullptr)
        {
            if (strcmp(rel_name, "*") == 0)
            {
                if (strcmp(attr_name, "*") != 0)
                {
                    debugPrint("SelectStatement:invalid field name while table is *. attr=%s\n", attr_name);
                    parse_session->setResponse("SELECT ERROR,SQL SYNTAX ERROR");
                    return Re::SchemaFieldMissing;
                }
                for (auto &t : tables_vec)
                    wildcardFields(t, fields_vec);
            }
            else
            {
                auto it = tables_map.find(std::string(rel_name));
                if (it == tables_map.end())
                {
                    debugPrint("SelectStatement:no such table in from list:%s\n", rel_name);
                    parse_session->setResponse("SELECT ERROR,NO SUCH TABLE");
                    return Re::SchemaFieldMissing;
                }
                Table *table = it->second;
                if (strcmp(attr_name, "*") == 0)
                    wildcardFields(table, fields_vec);
                else
                {
                    const FieldMeta *field_meta = table->getTableMeta().getField(attr_name);
                    if (field_meta == nullptr)
                    {
                        debugPrint("SelectStatement:no such field:%s in the table,invalid args\n", attr_name);
                        parse_session->setResponse("SELECT ERROR,NO SUCH FIELD IN TABLE");
                        return Re::SchemaFieldMissing;
                    }
                    fields_vec.emplace_back(table, field_meta);
                }
            }
        }
        else
        {
            if (tables_vec.size() != 1)
            {
                debugPrint("SelectStatement:not clearly given attrs\n");
                parse_session->setResponse("SELECT ERROR,ATTR GIVEN NOT CLEARLY");
                return Re::SchemaFieldMissing;
            }
            Table *table = tables_vec[0];
            const FieldMeta *field_meta = table->getTableMeta().getField(attr_name);
            if (field_meta == nullptr)
            {
                debugPrint("SelectStatement:no such field:%s in the table,invalid args\n", attr_name);
                parse_session->setResponse("SELECT ERROR,NO SUCH FIELD IN TABLE");
                return Re::SchemaFieldMissing;
            }
            fields_vec.emplace_back(table, field_meta);
        }
    }
    debugPrint("SelectStatement:got %d tables in from stmt and %d fields in query stmt\n",
               tables_vec.size(), fields_vec.size());
    Table *default_table = nullptr;
    if (tables_vec.size() == 1)
        default_table = tables_vec[0];
    Filter *filter = nullptr;
    Re r = Filter::createFilter(current_db, default_table, &tables_map, conditions_num_, conditions_, filter);
    if (r != Re::Success)
    {
        debugPrint("SelectStatement:create filter failed\n");
        return r;
    }
    filter_ = filter;
    tables_.swap(tables_vec);
    fields_.swap(fields_vec);
    return Re::Success;
}

void SelectStatement::destroy()
{
    for (int i = 0; i < table_names_num_; i++)
        delete[] table_names_[i];
    delete[] table_names_;
    for (int i = 0; i < attrs_num_; i++)
        attrs_[i].destroy();
    delete[] attrs_;
    for (int i = 0; i < conditions_num_; i++)
        conditions_[i].destroy();
    delete[] conditions_;
    delete filter_;
}

CreateTableStatement::CreateTableStatement(Query *query) : Statement(query->getScf()) { table_name_ = nullptr, attr_infos_ = nullptr, attr_infos_num_ = 0; }

void CreateTableStatement::init(Query *query)
{
    auto ctq = static_cast<CreateTableQuery *>(query);
    assert(this->getScf() == ScfCreateTable);
    attr_infos_num_ = ctq->getAttrNum();
    table_name_ = strNew(ctq->getRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = ctq->getAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i] = attr_infos[i];
}

Re CreateTableStatement::handle(Query *query, Session *parse_session)
{
    return Re::Success;
}

void CreateTableStatement::destroy()
{
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].destroy();
    delete[] attr_infos_;
}

InsertStatement::InsertStatement(Query *query) : Statement(query->getScf()), table_name_(nullptr), values_(nullptr), values_num_(0) {}

void InsertStatement::init(Query *query)
{
    auto iq = static_cast<InsertQuery *>(query);
    assert(this->getScf() == ScfInsert);
    values_num_ = iq->getValuesNum();
    table_name_ = strNew(iq->getRelName());
    values_ = new Value[values_num_];
    Value *values = iq->getValues();
    for (int i = 0; i < values_num_; i++)
        values_[i].copy(values[i]);
}

Re InsertStatement::handle(Query *query, Session *parse_session)
{
    Txn *txn = new Txn;
    parse_session->setTxn(txn);
    auto ps = static_cast<ParseSession *>(parse_session);
    DataBase *current_db = ps->getDb();
    if (current_db == nullptr or table_name_ == nullptr or values_num_ <= 0)
    {
        debugPrint("InsertStatement:invalid argument. db=%p, table_name=%p,values_num=%d\n",
                   current_db, table_name_, values_num_);
        parse_session->setResponse("INSERT ERROR,INVALID GIVEN ARGS.");
        return Re::InvalidArgument;
    }
    std::string table_name_str = std::string(table_name_);
    Table *table = current_db->getTable(table_name_str);
    if (table == nullptr)
    {
        debugPrint("InsertStatement:no such table. db=%s, table_name=%s\n",
                   current_db->getDbName().c_str(), table_name_);
        parse_session->setResponse("INSERT ERROR,NO SUCH TABLE.");
        return Re::SchemaTableNotExist;
    }
    const TableMeta &table_meta = table->getTableMeta();
    int table_sys_field_num = TableMeta::getSysFieldsNum();
    int table_user_field_num = table_meta.getFieldsNum() - table_sys_field_num;
    // values_num not equal to that in table_meta
    if (values_num_ != table_user_field_num)
    {
        debugPrint("InsertStatement:field num not equal to values num,values_num=%d,fields_num=%d\n",
                   values_num_, table_user_field_num);
        parse_session->setResponse("INSERT ERROR,VALUES NUM INVALID.");
        return Re::SchemaFieldMissing;
    }
    for (int i = 0; i < values_num_; i++)
    {
        const FieldMeta *field_meta = table_meta.getField(i + table_sys_field_num);
        if (values_[i].type != field_meta->getAttrType())
        {
            debugPrint(
                "InsertStatement:field getExprType mismatch. table=%s, field=%s, field getExprType=%d, value_type=%d\n",
                table_name_, field_meta->getFieldName().c_str(), field_meta->getAttrType(), values_[i].type);
            parse_session->setResponse("INSERT ERROR,VALUES TYPE INVALID.");
            return Re::SchemaFieldTypeMismatch;
        }
        // todo:advanced data check
        switch (values_[i].type)
        {
        default:
            break;
        }
    }
    return Re::Success;
}

void InsertStatement::destroy()
{
    delete[] table_name_;
    for (int i = 0; i < values_num_; i++)
        values_[i].destroy();
    delete[] values_;
}
