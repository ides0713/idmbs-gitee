#include "resolve_main.h"
#include <stdio.h>

RE ResolveMain::handle()
{
    ParseSession *ps = static_cast<ParseSession *>(parse_session_);
    Query *q = ps->getQuery();
    Statement::createStatement(q, stmt_);
    if (stmt_ == nullptr)
    {
        printf("ResolveMain:create statement failed\n");
        return RE::FAIL;
    }
    stmt_->initialize(q);
    stmt_->handle(q);
    resolve_session_ = new ResolveSession(parse_session_, stmt_);
    q->destroy();
    return RE::SUCCESS;
}

Session *ResolveMain::callBack()
{
    return resolve_session_;
}
