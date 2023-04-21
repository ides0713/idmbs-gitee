#pragma once

#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"
#include "global_main_manager.h"

class GlobalManagers
{
public:
    static void init();

    static GlobalMainManager &globalMainManager();

    static GlobalParamsManager &globalParamsManager();

    static GlobalDataBaseManager &globalDataBaseManager();

    static GlobalBufferPoolManager &globalBufferPoolManager();

    static void destroy();

private:
};