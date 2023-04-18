#include "parse_main.h"
#include "parse.h"

Re ParseMain::handle(const char *st)
{
    int rv = parse(st, query_);
    if (!rv)
    {
        query_->destroy();
        query_ = nullptr;
        setResponse("PARSE SQL FAILED.\n");
        return Re::SqlSyntax;
    }
    else
    {
        parse_session_ = new ParseSession(nullptr, nullptr, false, query_);
        return Re::Success;
    }
}

Session *ParseMain::callBack()
{
    return parse_session_;
}

void ParseMain::response()
{
    printf("%s", response_);
}
