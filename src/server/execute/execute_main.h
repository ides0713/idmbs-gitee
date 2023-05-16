#pragma once
#include "../common/base_main.h"// for BaseMain, Execute, MainType
#include "../common/re.h"       // for Re
class Statement;
class ExecuteMain : public BaseMain
{
public:
    ExecuteMain() : stmt_(nullptr) { SetType(MainType::Execute); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;

private:
    Re DoSelect(Statement *stmt);
    Re DoCreateTable(Statement *stmt);
    Re DoInsert(Statement *stmt);
    Re DoDelete(Statement *stmt);
    Re DoCreateIndex(Statement *stmt);

private:
    Statement *stmt_;
};