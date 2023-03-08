#include "resolve_main.h"
#include <stdio.h>
ResolveMain::ResolveMain()
{
    stmt_ = nullptr;
}
RE ResolveMain::execute(Query *query)
{
    Statement::createStatement(query, stmt_);
    if (stmt_ == nullptr)
    {
        printf("create statement failed\n");
        return RE::FAIL;
    }
    stmt_->initialize(query);
    stmt_->create(query);
    query->destroy();
}