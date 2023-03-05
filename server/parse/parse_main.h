#pragma once
#include "parse.h"
#include <stdio.h>
#include <assert.h>
#include "../../src/common_defs.h"


class Parse{
    public:
        Parse();
        ~Parse();
        RE parseMain(const char * st);
    private:
        Query* query_;
};