#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "resolve_defs.h"
#include "../common/re.h"
class ResolveMain {
public:
    explicit ResolveMain(Session *parse_session) : stmt_(nullptr), parse_session_(parse_session),
                                                   resolve_session_(nullptr) {}

    Re handle();

    Session *callBack();

private:
    Statement *stmt_;
    Session *parse_session_, *resolve_session_;
};