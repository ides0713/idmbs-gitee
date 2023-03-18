#include "resolve_main.h"
#include <stdio.h>
ResolveMain::ResolveMain(Session* session)
{
    parse_session_=session;
    stmt_ = nullptr;
}

RE ResolveMain::handle()
{
    ParseSession* ps=static_cast<ParseSession*>(parse_session_);
    Query* q=ps->getQuery();
    Statement::createStatement(q, stmt_);
    if (stmt_ == nullptr)
    {
        printf("ResolveMain:create statement failed\n");
        return RE::FAIL;
    }
    stmt_->initialize(q);
    stmt_->handle(q);
    resolve_session_=new ResolveSession(parse_session_,stmt_);
    q->destroy();
}

Session *ResolveMain::callBack()
{
    return resolve_session_;
}
