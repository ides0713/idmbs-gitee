#pragma once
#include "../common/base_main.h"
#include "../common/re.h"
#include "../common/server_defs.h"
class Query;
class StartMain;
class ParseMain : public BaseMain
{
public:
    ParseMain() : sql_(nullptr), query_(nullptr) { SetType(MainType::Parses); }
    Re Init(BaseMain *last_main) override;
    Re Handle() override;
    void Clear() override;
    void Destroy() override;
    Query *GetQuery() { return query_; }

private:
    char *sql_;
    Query *query_;
};