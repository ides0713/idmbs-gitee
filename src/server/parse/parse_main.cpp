#include "parse_main.h"
#include "parse.h"
#include "../common/start_main.h"

Re ParseMain::init(BaseMain *last_main)
{
    baseSet(*last_main);
    auto start_main = static_cast<StartMain *>(last_main);
    // last_main.
    sql_ = start_main->getSql();
    if (sql_ == nullptr or strlen(sql_) == 0)
        return Re::GenericError;
    return Re::Success;
}

Re ParseMain::handle()
{
    int rv = parse(sql_, query_);
    if (!rv)
    {
        query_->destroy();
        query_ = nullptr;
        GlobalMainManager &gmm = GlobalManagers::globalMainManager();
        gmm.setResponse("SQL PARSE ERROR\n");
        return Re::SqlSyntax;
    }
    return Re::Success;
}

void ParseMain::clear()
{
    if (query_ != nullptr)
    {
        query_->destroy();
        query_ = nullptr;
    }
    if (sql_ != nullptr)
        sql_ = nullptr;
}

void ParseMain::destroy()
{
}

// Re ParseMain::handle(const char *st)
// {
//     int rv = parse(st, query_);
//     if (!rv)
//     {
//         query_->destroy();
//         query_ = nullptr;
//         setResponse("PARSE SQL FAILED.\n");
//         return Re::SqlSyntax;
//     }
//     else
//     {
//         parse_session_ = new ParseSession(nullptr, nullptr, false, query_);
//         return Re::Success;
//     }
// }

// Session *ParseMain::callBack()
// {
//     return parse_session_;
// }
