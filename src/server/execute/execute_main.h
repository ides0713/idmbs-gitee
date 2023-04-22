#pragma once
#include "../common/base_main.h"
#include "../common/re.h"
#include "../common/server_defs.h"
#include "../resolve/resolve_defs.h"
#include "../storage/clog_manager.h"
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

private:
    Statement *stmt_;
};