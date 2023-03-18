#pragma once
#include "../common/server_defs.h"
#include "../common/session.h"
#include "resolve_defs.h"

class ResolveMain
{
public:
    ResolveMain(Session * parse_session);
    RE handle();
    Session* callBack();
private:
    Statement *stmt_;
    Session* parse_session_,*resolve_session_;
};