#pragma once

#include "../common/base_main.h"
#include "../common/re.h"
#include "../common/server_defs.h"
#include "resolve_defs.h"
#include <unordered_map>

class ResolveMain : public BaseMain {
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