#include "parse_main.h"
#include "parse.h"
#include "parse_defs.h"
#include <stdio.h>
#include <assert.h>
ParseMain::ParseMain()
{
    query_ = nullptr;
    parse_session_=nullptr;
}
RE ParseMain::handle(const char *st)
{
    int rv = parse(st, query_);
    if (!rv)
    {
        query_->destroy();
        query_ = nullptr;
        return RE::FAIL;
    }
    else{
        parse_session_=new ParseSession(nullptr,nullptr,false,query_);
        return RE::SUCCESS;
    }
}

Session *ParseMain::callBack()
{
    return parse_session_;
}
