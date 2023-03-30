#include "global_managers.h"

void GlobalManagers::initialize() {
    globalParamsManager().initialize();
    globalDataBaseManager().initialize();
    globalBufferPoolManager().initialize();
}

void GlobalManagers::destroy() {
    globalParamsManager().destroy();
    globalDataBaseManager().destroy();
    globalBufferPoolManager().destroy();
}

GlobalParamsManager &GlobalManagers::globalParamsManager() {
    static GlobalParamsManager instance;
    return instance;
}

GlobalDataBaseManager &GlobalManagers::globalDataBaseManager() {
    static GlobalDataBaseManager instance;
    return instance;
}

GlobalBufferPoolManager &GlobalManagers::globalBufferPoolManager() {
    static GlobalBufferPoolManager instance;
    return instance;
}
