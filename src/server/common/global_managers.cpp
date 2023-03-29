#include "global_managers.h"

void GlobalManagers::initialize() {
    debugPrint("GlobalManagers:initialize begin\n");
    getGlobalPramsManager().initialize();
    getDataBaseManager().initialize();
    getBufferPoolManager().initialize();
}

void GlobalManagers::destroy() {
    getGlobalPramsManager().destroy();
    getDataBaseManager().destroy();
    getBufferPoolManager().destroy();
}

GlobalParamsManager &GlobalManagers::getGlobalPramsManager() {
    static GlobalParamsManager instance;
    return instance;
}

DataBaseManager &GlobalManagers::getDataBaseManager() {
    static DataBaseManager instance;
    return instance;
}

BufferPoolManager &GlobalManagers::getBufferPoolManager() {
    static BufferPoolManager instance;
    return instance;
}
