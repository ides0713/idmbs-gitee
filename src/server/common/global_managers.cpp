#include "global_managers.h"

void GlobalManagers::initialize() {
    globalParamsManager().initialize();
    dataBaseManager().initialize();
    bufferPoolManager().initialize();
}

void GlobalManagers::destroy() {
    globalParamsManager().destroy();
    dataBaseManager().destroy();
    bufferPoolManager().destroy();
}

GlobalParamsManager &GlobalManagers::globalParamsManager() {
    static GlobalParamsManager instance;
    return instance;
}

DataBaseManager &GlobalManagers::dataBaseManager() {
    static DataBaseManager instance;
    return instance;
}

BufferPoolManager &GlobalManagers::bufferPoolManager() {
    static BufferPoolManager instance;
    return instance;
}
