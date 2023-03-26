#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"

class ExecuteMain {
public:
    explicit ExecuteMain(Session *resolve_session) : resolve_session_(resolve_session), execute_session_(nullptr) {}

    RE handle();

    Session *callBack();

private:
    RE doSelect(Statement *stmt);

    RE doCreateTable(Statement *stmt);

    Session *resolve_session_, *execute_session_;
};