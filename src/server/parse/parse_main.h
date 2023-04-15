#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "../common/re.h"

class Query;

class ParseMain {
public:
    ParseMain() : query_(nullptr), parse_session_(nullptr) {}

    Re handle(const char *st);

    Session *callBack();

    void setResponse(const char *msg) { strcpy(response_, msg); }

    void response();

private:
    Query *query_;
    char response_[MAX_MSG_LENGTH];
    Session *parse_session_;
};