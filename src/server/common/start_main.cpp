#include "start_main.h"

#include "../parse/parse_defs.h"                        // for StrNew
#include "../../common/common_defs.h"  // for DebugPrint

Re StartMain::Init(BaseMain *last_main) {
    return Re::Success;
}
Re StartMain::Handle() {
    return Re::Success;
}
void StartMain::Clear() {
    if (sql_ != nullptr) {
        delete[] sql_;
        sql_ = nullptr;
    }
}
void StartMain::Destroy() {
    if (sql_ != nullptr) {
        delete[] sql_;
        sql_ = nullptr;
    }
}
char *StartMain::GetSql() {
    return sql_;
}
void StartMain::SetSql(const char *sql) {
    if (sql_ != nullptr) {
        delete[] sql_;
        sql_ = nullptr;
    }
    sql_ = StrNew(sql);
    DebugPrint("\nsetted sql is         %s\n\n", sql_);
}
