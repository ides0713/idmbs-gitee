#pragma once
#include "base_main.h"  // for BaseMain, MainType, Start
#include "re.h"         // for Re

class StartMain : public BaseMain
{
public:
    StartMain() : sql_(nullptr) { SetType(MainType::Start); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;
    char *GetSql();
    void SetSql(const char *sql);

private:
    char *sql_;
};