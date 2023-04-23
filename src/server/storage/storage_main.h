#pragma once
#include "../common/base_main.h"  // for BaseMain, MainType, Storage
#include "../common/re.h"         // for Re

class StorageMain : public BaseMain
{
public:
    //    explicit ExecuteMain(Session *resolve_session) : resolve_session_(resolve_session), execute_session_(nullptr) {}
    StorageMain() { SetType(MainType::Storage); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;

private:
};