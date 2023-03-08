#pragma once
#include "../parse/parse_defs.h"
#include "../../src/common_defs.h"
#include <unistd.h>
#include <dirent.h>
// bin---
//      dbdir
//          dbfile
//          datefile
//          indexfile


class StorageMain{
    public:
    StorageMain(Query * query){
        query_=query;
    }
    RE handle();
    void nextP();
    private:
    DIR * findBin();
    DIR * findDBDir();
    FILE* findDBFile(DIR* bin_dir);
    Query* query_;
};