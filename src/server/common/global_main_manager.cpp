#include "global_main_manager.h"
#include "../execute/execute_main.h"
#include "../parse/parse_main.h"
#include "../resolve/resolve_main.h"
#include "../storage/storage_main.h"
#include "start_main.h"
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
            for (int j = 0; j <= i; j++)
                mains_[i]->Clear();
            return;
        }
        DebugPrint("%s main init succeeded\n", StrMainType(mains_[i]->GetType()).c_str());
        r = mains_[i]->Handle();
        if (r != Re::Success) {
            DebugPrint("%s main handle failed\n", StrMainType(mains_[i]->GetType()).c_str());
            Response();
            for (int j = 0; j <= i; j++)
                mains_[i]->Clear();
            return;
        }
        DebugPrint("%s main handle succeeded\n", StrMainType(mains_[i]->GetType()).c_str());
    }
    DoneResponse();
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
    va_list var_list;
    va_start(var_list, format);
    char *a = new char[100];
    vsprintf(a, format, var_list);
    response_.assign(a);
    delete[] a;
    va_end(var_list);
}

void GlobalMainManager::Response() {
    if (response_.empty())
        SetResponse("SQL ended and response is empty\n");
    printf("%s", response_.c_str());
}

void GlobalMainManager::DoneResponse() {
    if (response_.empty())
        printf("SQL SUCCEEDED\n");
    else
        Response();
}

void GlobalMainManager::Clear() {
    for (int i = 0; i < mains_.size(); i++)
        mains_[i]->Clear();
}
