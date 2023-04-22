#include "parse_main.h"
#include "../common/global_managers.h"
#include "../common/start_main.h"
#include "parse.h"
Re ParseMain::Init(BaseMain *last_main) {
    BaseSet(*last_main);
    auto start_main = static_cast<StartMain *>(last_main);
    // last_main.
    sql_ = start_main->GetSql();
    if (sql_ == nullptr or strlen(sql_) == 0)
        return Re::GenericError;
    return Re::Success;
}
Re ParseMain::Handle() {
    int rv = Parse(sql_, query_);
    if (!rv) {
        GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
        gmm.SetResponse("SQL PARSE ERROR\n");
        return Re::SqlSyntax;
    }
    return Re::Success;
}
void ParseMain::Clear() {
    if (query_ != nullptr) {
        query_->Destroy();
        query_ = nullptr;
    }
    if (sql_ != nullptr)
        sql_ = nullptr;
}
void ParseMain::Destroy() {
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
