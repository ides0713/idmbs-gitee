#pragma once
#include "resolve_defs.h"
#include "../../src/server_defs.h"
class ResolveMain
{
public:
    ResolveMain();
    RE handle(Query *query);

private:
    Statement *stmt_;
};