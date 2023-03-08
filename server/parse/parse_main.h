#pragma once
#include "../../src/common_defs.h"
#include "parse_defs.h"

class ParseMain{
    public:
        ParseMain();
        RE execute(const char * st);
        Query* nextP(){return query_;}
    private:
        Query* query_;
};