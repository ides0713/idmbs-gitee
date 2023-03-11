#pragma once
#include "../parse/parse.h"
#include "../../src/server_defs.h"
#include <unistd.h>
#include <dirent.h>
#include "database.h"
// bin---
//      dbdir
//          dbfile
//          datefile
//          indexfile

class StorageMain
{
public:
    StorageMain();
    RE handle(Query *query);
    void nextP();

private:
    DIR *findBin();
    DIR *findDBDir();
    FILE *findDBFile(DIR *bin_dir);
};