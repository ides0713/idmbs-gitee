#pragma once
#include "../common/base_main.h"  // for BaseMain, MainType, Resolve
#include "../common/re.h"         // for Re

class Query;
class Statement;

class ResolveMain : public BaseMain
{
public:
    ResolveMain() : query_(nullptr), stmt_(nullptr) { SetType(MainType::Resolve); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;
    Statement *GetStmt() { return stmt_; }

private:
    Query *query_;
    Statement *stmt_;
};