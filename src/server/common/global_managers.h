#pragma once

#include "../storage/buffer_pool.h"
#include "../storage/database.h"
#include "global_main_manager.h"
#include "server_defs.h"

class GlobalManagers {
public:
    static void Init();

    static GlobalMainManager &GetGlobalMainManager();

    static GlobalParamsManager &GetGlobalParamsManager();

    static GlobalDataBaseManager &GetGlobalDataBaseManager();

    static GlobalBufferPoolManager &GetGlobalBufferPoolManager();

    static void Destroy();
};