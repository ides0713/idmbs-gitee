#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "../common/re.h"
#include "../storage/clog_manager.h"

class ExecuteMain
{
public:
    explicit ExecuteMain(Session *resolve_session) : resolve_session_(resolve_session), execute_session_(nullptr) {}

    Re handle();

    Session *callBack();

    void response();

private:
    Re doSelect(Statement *stmt);

    Re doCreateTable(Statement *stmt);

    Re doInsert(Statement *stmt);

    Session *resolve_session_, *execute_session_;
};