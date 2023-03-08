#pragma once
#include "../parse/parse_defs.h"
#include "../../src/common_defs.h"
#include <vector>
class Statement
{
public:
    Statement(SqlCommandFlag flag) : flag_(flag) {}
    virtual void initialize(Query* query) = 0;
    virtual void create(Query* query)=0;
    virtual void destroy() = 0;
    SqlCommandFlag getSCF() { return flag_; }
    static void createStatement(Query *const query, Statement *&stmt);
private:
    SqlCommandFlag flag_;
};
class SelectStatement : public Statement
{
public:
    SelectStatement(Query *query) : Statement(query->getSCF()){}
    void initialize(Query* query) override;
    void create(Query* query) override;
    void destroy() override;
private:
};

class CreateTableStatement : public Statement
{
public:
    CreateTableStatement(Query *query) : Statement(query->getSCF()){}
    void initialize(Query * query) override;
    void create(Query* query) override;
    void destroy() override;
private:
    char * table_name_;
    AttrInfo* attr_infos_;
};
