#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "../common/re.h"
#include "../storage/clog_manager.h"
#include "../common/base_main.h"

class ExecuteMain : public BaseMain
{
public:
    ExecuteMain() : stmt_(nullptr) { setType(MainType::Execute); }
    Re init(BaseMain *last_main) override;
    Re handle() override;
    void clear() override;
    void destroy() override;

private:
    Re doSelect(Statement *stmt);

    Re doCreateTable(Statement *stmt);

    Re doInsert(Statement *stmt);

private:
    Statement *stmt_;
};