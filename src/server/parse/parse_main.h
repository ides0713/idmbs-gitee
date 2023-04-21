#pragma once

#include "../common/server_defs.h"
#include "../common/session.h"
#include "../common/re.h"
#include "../common/base_main.h"

class Query;
class StartMain;
class ParseMain : public BaseMain
{
public:
    ParseMain() : sql_(nullptr), query_(nullptr) { setType(MainType::Parse); }
    Re init(BaseMain *last_main) override;
    Re handle() override;
    void clear() override;
    void destroy() override;
    Query *getQuery() { return query_; }

private:
    char *sql_;
    Query *query_;
};