#pragma once
class GlobalBufferPoolManager;
class GlobalDataBaseManager;
class GlobalMainManager;
class GlobalParamsManager;

class GlobalManagers
{
public:
    static void Init();
    static GlobalMainManager &GetGlobalMainManager();
    static GlobalParamsManager &GetGlobalParamsManager();
    static GlobalDataBaseManager &GetGlobalDataBaseManager();
    static GlobalBufferPoolManager &GetGlobalBufferPoolManager();
    static void Destroy();
};