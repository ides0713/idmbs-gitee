#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <vector>
#include "../common/re.h"
#include "../storage/database.h"
#include "../common/global_managers.h"

class Session;

enum StatementFlag {
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

    virtual StatementFlag getType() = 0;

    SqlCommandFlag getScf() { return flag_; }

    static void createStatement(Query *const query, Session *parse_session, Statement *&stmt);

private:
    SqlCommandFlag flag_;
};

class SelectStatement : public Statement {
public:
    explicit SelectStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query, Session *parse_session) override;

    void destroy() override;

    StatementFlag getType() override { return StatementFlag::Select; }

private:
};

class CreateTableStatement : public Statement {
public:
    CreateTableStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query, Session *parse_session) override;

    void destroy() override;

    StatementFlag getType() override { return StatementFlag::CreateTable; }

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

    StatementFlag getType() override { return StatementFlag::Insert; }

    const char *getTableName() { return table_name_; }

    const Value *getValues() { return values_; }

    [[nodiscard]] int getValuesNum() const { return values_num_; }

private:
    char *table_name_;
    Value *values_;
    int values_num_;
};
