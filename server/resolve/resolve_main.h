#pragma once
#include "../../src/server_defs.h"
#include "resolve_defs.h"

class ResolveMain
{
public:
    ResolveMain();
    RE handle(Query *query);

private:
    Statement *stmt_;
};