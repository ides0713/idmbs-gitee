#pragma once

#include "../common/global_managers.h"
#include "../common/re.h"
#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include "../storage/database.h"
#include "filter.h"
#include <vector>
class Session;
class ResolveMain;
enum StatementType {
    Select = 0,
    CreateTable,
    Insert
};

class Statement {
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

class SelectStatement : public Statement {
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

class CreateTableStatement : public Statement {
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
};

class InsertStatement : public Statement {
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
