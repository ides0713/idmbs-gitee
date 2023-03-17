#include "managers_initializer.h"

ManagerInitializer &ManagerInitializer::getInstance()
{
    static ManagerInitializer instance;
    return instance;
}

void ManagerInitializer::handle()
{
    GlobalParamsManager::getInstance().initialize();
    DataBaseManager::getInstance().initialize();
    BufferPoolManager::getInstance().initialize();
}

void ManagerInitializer::destroy()
{
    GlobalParamsManager::getInstance().destroy();
    DataBaseManager::getInstance().destroy();
    BufferPoolManager::getInstance().destroy();
}
