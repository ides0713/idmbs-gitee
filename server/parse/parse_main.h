#pragma once
#include "parse.h"
#include <stdio.h>
#include <assert.h>
#include "../../src/common_defs.h"


class Parse{
    public:
        Parse();
        ~Parse();
        returnInfo* parseMain(const char * st);
    private:
        QueryInfo* query_;
        returnInfo* return_info_;
};