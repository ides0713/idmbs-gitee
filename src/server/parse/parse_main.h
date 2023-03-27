#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"

class Query;

class ParseMain {
public:
    ParseMain() : query_(nullptr), parse_session_(nullptr) {}

    Re handle(const char *st);

    Session *callBack();

private:
    Query *query_;
    Session *parse_session_;
};