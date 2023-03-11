#pragma once
#include "../../src/server_defs.h"

class Statement;

class ResolveMain
{
public:
    ResolveMain();
    RE handle(Query *query);

private:
    Statement *stmt_;
};