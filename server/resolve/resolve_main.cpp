#include "resolve_main.h"
#include <stdio.h>
#include "resolve_defs.h"
ResolveMain::ResolveMain()
{
    stmt_ = nullptr;
}

RE ResolveMain::handle(Query *query)
{
    Statement::createStatement(query, stmt_);
    if (stmt_ == nullptr)
    {
        printf("create statement failed\n");
        return RE::FAIL;
    }
    stmt_->initialize(query);
    stmt_->handle(query);
    query->destroy();
}