#include "global_managers.h"

#include "/home/ubuntu/idbms/src/common/common_defs.h"          // for Debug...
#include "/home/ubuntu/idbms/src/server/storage/buffer_pool.h"  // for Globa...
#include "/home/ubuntu/idbms/src/server/storage/database.h"     // for Globa...
#include "global_main_manager.h"                                // for Globa...
#include "server_defs.h"                                        // for Globa...

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
