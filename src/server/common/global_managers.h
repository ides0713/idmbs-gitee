#pragma once

#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"

class GlobalManagers {
public:
    static void init();

    static GlobalParamsManager &globalParamsManager();

    static GlobalDataBaseManager &globalDataBaseManager();

    static GlobalBufferPoolManager &globalBufferPoolManager();

    static void destroy();

private:
};