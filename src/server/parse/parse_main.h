#pragma once
#include "../common/server_defs.h"
#include "../common/session.h"
class Query;

class ParseMain
{
public:
    ParseMain();
    ~ParseMain();
    RE handle(const char *st);
    std::pair<Query*,Session> callBack();
    Query *getQuery() { return query_; }
private:
    Query *query_;
};