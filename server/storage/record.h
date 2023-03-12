#pragma once
#include "../parse/parse_defs.h"
class Table;
class Record{
    public:
        Record();
        ~Record();
        
    private:
        char *data_;
};
