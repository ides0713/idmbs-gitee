#include "global_managers_initializer.h"

GlobalManagersInitializer &GlobalManagersInitializer::getInstance() {
    static GlobalManagersInitializer instance;
    return instance;
}

void GlobalManagersInitializer::handle() {
    GlobalParamsManager::getInstance().initialize();
    DataBaseManager::getInstance().initialize();
    BufferPoolManager::getInstance().initialize();
}

void GlobalManagersInitializer::destroy() {
    GlobalParamsManager::getInstance().destroy();
    DataBaseManager::getInstance().destroy();
    BufferPoolManager::getInstance().destroy();
}
