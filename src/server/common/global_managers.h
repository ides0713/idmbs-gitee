#pragma once
#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"

class GlobalManagers {
public:
    static void initialize();

    static GlobalParamsManager &getGlobalPramsManager();

    static DataBaseManager &getDataBaseManager();

    static BufferPoolManager &getBufferPoolManager();

    static void destroy();
private:
//    static GlobalParamsManager gpm_;
//    static DataBaseManager dbm_;
//    static BufferPoolManager bpm_;
};