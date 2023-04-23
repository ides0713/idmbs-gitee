#pragma once
#include "../common/base_main.h"  // for BaseMain, MainType, Parses
#include "../common/re.h"         // for Re

class Query;

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