#include "global_managers.h"

void GlobalManagers::init()
{
    globalParamsManager().init();
    globalDataBaseManager().init();
    globalBufferPoolManager().init();
    globalMainManager().init();
    debugPrint("GlobalManagers initialized done\n");
}

GlobalMainManager &GlobalManagers::globalMainManager()
{
    static GlobalMainManager instance;
    return instance;
}

void GlobalManagers::destroy()
{
    globalParamsManager().destroy();
    globalDataBaseManager().destroy();
    globalBufferPoolManager().destroy();
    globalMainManager().destroy();
}

GlobalParamsManager &GlobalManagers::globalParamsManager()
{
    static GlobalParamsManager instance;
    return instance;
}

GlobalDataBaseManager &GlobalManagers::globalDataBaseManager()
{
    static GlobalDataBaseManager instance;
    return instance;
}

GlobalBufferPoolManager &GlobalManagers::globalBufferPoolManager()
{
    static GlobalBufferPoolManager instance;
    return instance;
}
