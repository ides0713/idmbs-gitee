#include "parse_main.h"

Parse::Parse()
{
    query_ = nullptr;
}
Parse::~Parse()
{
    if(query_!=nullptr)
        query_->destroy();
}
RE Parse::parseMain(const char *st)
{
    int rv = parse(st, query_);
    if (!rv)
        return RE::FAIL;
    else
        return RE::SUCCESS;
}
