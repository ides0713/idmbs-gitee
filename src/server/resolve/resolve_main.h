#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "resolve_defs.h"
#include "../common/re.h"
#include <unordered_map>
#include "../common/base_main.h"

class ResolveMain : public BaseMain
{
public:
    ResolveMain() : query_(nullptr), stmt_(nullptr) { setType(MainType::Resolve); }
    Re init(BaseMain *last_main) override;
    Re handle() override;
    void clear() override;
    void destroy() override;
    Statement *getStmt() { return stmt_; }

private:
    Query *query_;
    Statement *stmt_;
};