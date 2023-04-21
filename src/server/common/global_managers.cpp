#include "global_managers.h"

void GlobalManagers::Init() {
    GetGlobalParamsManager().Init();
    GetGlobalDataBaseManager().Init();
    GetGlobalBufferPoolManager().Init();
    GetGlobalMainManager().Init();
    DebugPrint("GlobalManagers initialized done\n");
}

GlobalMainManager &GlobalManagers::GetGlobalMainManager() {
    static GlobalMainManager instance;
    return instance;
}

void GlobalManagers::Destroy() {
    GetGlobalParamsManager().Destroy();
    GetGlobalDataBaseManager().Destroy();
    GetGlobalBufferPoolManager().Destroy();
    GetGlobalMainManager().Destroy();
}

GlobalParamsManager &GlobalManagers::GetGlobalParamsManager() {
    static GlobalParamsManager instance;
    return instance;
}

GlobalDataBaseManager &GlobalManagers::GetGlobalDataBaseManager() {
    static GlobalDataBaseManager instance;
    return instance;
}

GlobalBufferPoolManager &GlobalManagers::GetGlobalBufferPoolManager() {
    static GlobalBufferPoolManager instance;
    return instance;
}
