#pragma once

#include <unistd.h>
#include <dirent.h>
#include "database.h"
#include "../common/re.h"
#include "../common/session.h"
#include "../parse/parse.h"
#include "../common/server_defs.h"
#include "../resolve/resolve_defs.h"

class StorageMain {
public:
//    explicit ExecuteMain(Session *resolve_session) : resolve_session_(resolve_session), execute_session_(nullptr) {}
    explicit StorageMain(Session *execute_session) : execute_session_(execute_session), storage_session_(nullptr) {}

    Re handle();

    void response();

private:
    Session *execute_session_, *storage_session_;
};