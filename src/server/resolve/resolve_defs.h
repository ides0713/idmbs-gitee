#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <vector>
#include "../common/re.h"

enum StatementFlag {
    Select = 0,
    CreateTable,
};

class Statement {
public:
    explicit Statement(SqlCommandFlag flag) : flag_(flag) {}

    virtual void init(Query *query) = 0;

    virtual Re handle(Query *query) = 0;

    virtual void destroy() = 0;

    virtual StatementFlag getType() = 0;

    SqlCommandFlag getScf() { return flag_; }

    static void createStatement(Query *const query, Statement *&stmt);

private:
    SqlCommandFlag flag_;
};

class SelectStatement : public Statement {
public:
    explicit SelectStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query) override;

    void destroy() override;

    StatementFlag getType() override { return StatementFlag::Select; }

private:
};

class CreateTableStatement : public Statement {
public:
    explicit CreateTableStatement(Query *query);

    void init(Query *query) override;

    Re handle(Query *query) override;

    void destroy() override;

    StatementFlag getType() override { return StatementFlag::CreateTable; }

    const char *getTableName() { return table_name_; }

    const AttrInfo *getAttrInfos() { return attr_infos_; }

    [[nodiscard]] const size_t getAttrInfosNum() const { return attr_infos_num_; }

private:
    char *table_name_;
    AttrInfo *attr_infos_;
    size_t attr_infos_num_;
};
