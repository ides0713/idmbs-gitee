#include "global_managers_initializer.h"

GlobalManagersManager &GlobalManagersManager::getInstance() {
    static GlobalManagersManager instance;
    return instance;
}

void GlobalManagersManager::handle() {
    GlobalParamsManager::getInstance().initialize();
    DataBaseManager::getInstance().initialize();
    BufferPoolManager::getInstance().initialize();
}

void GlobalManagersManager::destroy() {
    GlobalParamsManager::getInstance().destroy();
    DataBaseManager::getInstance().destroy();
    BufferPoolManager::getInstance().destroy();
}
