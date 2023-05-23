#pragma once
#include "../common/re.h"       // for Re
#include "../parse/parse_defs.h"// for SqlCommandFlag
#include "../storage/field.h"   // for Field
#include <vector>               // for vector
class ResolveMain;
class Filter;
class Table;
enum StatementType
{
    Select = 0,
    CreateTable,
    Insert,
    Delete,
    CreateIndex,
    DropTable,
};
class Statement
{
public:
    explicit Statement(SqlCommandFlag flag) : flag_(flag) {}
    virtual void Init(Query *query) = 0;
    virtual Re Handle(Query *query, ResolveMain *resolve_main) = 0;
    virtual void Destroy() = 0;
    virtual StatementType GetType() = 0;
    SqlCommandFlag GetScf() { return flag_; }
    static void CreateStatement(Query *const query, Statement *&stmt);

private:
    SqlCommandFlag flag_;
};
class SelectStatement : public Statement
{
public:
    explicit SelectStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::Select; }
    void SetFilter(Filter *filter) { filter_ = filter; }
    Filter *GetFilter() { return filter_; }
    const std::vector<Table *> *GetTables() { return &tables_; }
    const std::vector<Field> *GetFields() { return &fields_; }

private:
    char **table_names_;
    int table_names_num_;
    RelAttr *attrs_;
    int attrs_num_;
    Condition *conditions_;
    int conditions_num_;
    Filter *filter_;
    std::vector<Table *> tables_;
    std::vector<Field> fields_;
};
class CreateTableStatement : public Statement
{
public:
    CreateTableStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::CreateTable; }
    const char *GetTableName() { return table_name_; }
    const AttrInfo *GetAttrInfos() { return attr_infos_; }
    [[nodiscard]] int GetAttrInfosNum() const { return attr_infos_num_; }

private:
    char *table_name_;
    AttrInfo *attr_infos_;
    int attr_infos_num_;

private:
    bool IsAllFieldsValid();
    bool IsFieldValid(const AttrInfo &attr);
};
class InsertStatement : public Statement
{
public:
    InsertStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::Insert; }
    const char *GetTableName() { return table_name_; }
    const Value *GetValues() { return values_; }
    [[nodiscard]] int GetValuesNum() const { return values_num_; }

private:
    char *table_name_;
    Value *values_;
    int values_num_;
};
class DeleteStatement : public Statement
{
public:
    explicit DeleteStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::Delete; }
    int GetContionsNum() const { return conditions_num_; }
    const char *GetTableName() { return table_name_; }
    const Condition *GetConditions() { return conditions_; }
    Filter *GetFilter() { return filter_; }
    Table *GetTable() { return table_; }

private:
    int conditions_num_;
    char *table_name_;
    Condition *conditions_;
    Filter *filter_;
    Table *table_;
};
class CreateIndexStatement : public Statement
{
public:
    explicit CreateIndexStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::CreateIndex; }
    const char *GetIndexName() { return index_name_; }
    RelAttr *GetAttr() { return attr_; }

private:
    char *index_name_;
    RelAttr *attr_;
};
class DropTableStatement : public Statement
{
public:
    explicit DropTableStatement(Query *query);
    void Init(Query *query) override;
    Re Handle(Query *query, ResolveMain *resolve_main) override;
    void Destroy() override;
    StatementType GetType() override { return StatementType::DropTable; }
    const char *GetTableName() { return table_name_; }

private:
    char *table_name_;
};