#include "managers_initializer.h"

GlobalManagerInitializer &GlobalManagerInitializer::getInstance()
{
    static GlobalManagerInitializer instance;
    return instance;
}

void GlobalManagerInitializer::handle()
{
    GlobalParamsManager::getInstance().initialize();
    DataBaseManager::getInstance().initialize();
    BufferPoolManager::getInstance().initialize();
}

void GlobalManagerInitializer::destroy()
{
    GlobalParamsManager::getInstance().destroy();
    DataBaseManager::getInstance().destroy();
    BufferPoolManager::getInstance().destroy();
}
