#pragma once
#include "../../src/common_defs.h"
#include "parse_defs.h"

class ParseMain{
    public:
        ParseMain();
        RE handle(const char * st);
        Query* getQuery(){return query_;}
    private:
        Query* query_;
};