#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <vector>
#include "../common/re.h"
#include "../storage/database.h"
#include "../common/global_managers.h"
#include "filter.h"

class Session;

enum StatementType {
    Select = 0,
    CreateTable,
    Insert
};

class Statement {
public:
    explicit Statement(SqlCommandFlag flag) : flag_(flag) {}

    virtual void init(Query *query) = 0;

    virtual Re handle(Query *query, Session *parse_session) = 0;

    virtual void destroy() = 0;

    virtual StatementType getType() = 0;

    SqlCommandFlag getScf() { return flag_; }

    static void createStatement(Query *const query, Statement *&stmt);

private:
    SqlCommandFlag flag_;
};

class SelectStatement : public Statement {
public:
    explicit SelectStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query, Session *parse_session) override;

    void destroy() override;

    StatementType getType() override { return StatementType::Select; }

    void setFilter(Filter *filter) { filter_ = filter; }

    Filter *getFilter() { return filter_; }

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

    void init(Query *query) override;

    Re handle(Query *query, Session *parse_session) override;

    void destroy() override;

    StatementType getType() override { return StatementType::CreateTable; }

    const char *getTableName() { return table_name_; }

    const AttrInfo *getAttrInfos() { return attr_infos_; }

    [[nodiscard]] int getAttrInfosNum() const { return attr_infos_num_; }

private:
    char *table_name_;
    AttrInfo *attr_infos_;
    int attr_infos_num_;
};

class InsertStatement : public Statement {
public:
    InsertStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query, Session *parse_session) override;

    void destroy() override;

    StatementType getType() override { return StatementType::Insert; }

    const char *getTableName() { return table_name_; }

    const Value *getValues() { return values_; }

    [[nodiscard]] int getValuesNum() const { return values_num_; }

private:
    char *table_name_;
    Value *values_;
    int values_num_;
};
