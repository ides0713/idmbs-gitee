#include "global_main_manager.h"
#include "../../common/common_defs.h"// for DebugPrint
#include "../execute/execute_main.h" // for ExecuteMain
#include "../parse/parse_main.h"     // for ParseMain
#include "../resolve/resolve_main.h" // for ResolveMain
#include "../storage/storage_main.h" // for StorageMain
#include "base_main.h"               // for StrMainType
#include "re.h"                      // for Re, Success
#include "start_main.h"              // for StartMain
#include <cstdarg>                   // for va_end, va_list
#include <stdio.h>                   // for printf, vsprintf
void GlobalMainManager::Init() {
    BaseMain *start_main = new StartMain();
    mains_.push_back(start_main);
    BaseMain *parse_main = new ParseMain();
    mains_.push_back(parse_main);
    BaseMain *resolve_main = new ResolveMain();
    mains_.push_back(resolve_main);
    BaseMain *execute_main = new ExecuteMain();
    mains_.push_back(execute_main);
    BaseMain *storage_main = new StorageMain();
    mains_.push_back(storage_main);
}
void GlobalMainManager::Handle(const char *sql) {
    static_cast<StartMain *>(mains_[0])->SetSql(sql);
    for (int i = 1; i < mains_.size(); i++) {
        Re r = mains_[i]->Init(mains_[i - 1]);
        if (r != Re::Success) {
            DebugPrint("%s main init failed\n", StrMainType(mains_[i]->GetType()).c_str());
            Response();
            Clear();
            return;
        }
        DebugPrint("%s main init succeeded\n", StrMainType(mains_[i]->GetType()).c_str());
        r = mains_[i]->Handle();
        if (r != Re::Success) {
            DebugPrint("%s main handle failed\n", StrMainType(mains_[i]->GetType()).c_str());
            Response();
            Clear();
            return;
        }
        DebugPrint("%s main handle succeeded\n", StrMainType(mains_[i]->GetType()).c_str());
    }
    Response();
    Clear();
}
void GlobalMainManager::Destroy() {
    for (int i = 0; i < mains_.size(); i++)
        mains_[i]->Destroy();
    mains_.clear();
}
void GlobalMainManager::SetResponse(std::string str) {
    response_.assign(str);
}
void GlobalMainManager::SetResponse(const char *format, ...) {
    if (response_.empty()) {
        va_list var_list;
        va_start(var_list, format);
        char *a = new char[100];
        vsprintf(a, format, var_list);
        response_.assign(a);
        delete[] a;
        va_end(var_list);
    }
}
void GlobalMainManager::Response() {
    if (response_.empty())
        SetResponse("SQL ENDED AND RESPONSE IS EMPTY\n");
    printf("%s", response_.c_str());
}
bool GlobalMainManager::IsResponseSet() {
    return !response_.empty();
}
void GlobalMainManager::ClearResponse() {
    response_.clear();
}
void GlobalMainManager::Clear() {
    for (int i = 0; i < mains_.size(); i++)
        mains_[i]->Clear();
    response_.clear();
}
