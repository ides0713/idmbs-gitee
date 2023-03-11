#pragma once
#include "../../src/server_defs.h"

class Query;

class ParseMain
{
public:
    ParseMain();
    RE handle(const char *st);
    Query *getQuery() { return query_; }
private:
    Query *query_;
};