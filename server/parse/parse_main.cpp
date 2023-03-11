#include "parse_main.h"
#include "parse.h"
#include "parse_defs.h"
#include <stdio.h>
#include <assert.h>
ParseMain::ParseMain()
{
    query_ = nullptr;
}
RE ParseMain::handle(const char *st)
{
    int rv = parse(st, query_);
    if (!rv)
    {
        query_->destroy();
        query_ = nullptr;
        return RE::FAIL;
    }
    else
        return RE::SUCCESS;
}
