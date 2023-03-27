#pragma once

#include "../parse/parse.h"
#include "../common/server_defs.h"
#include "../resolve/resolve_defs.h"
#include <unistd.h>
#include <dirent.h>
#include "database.h"

class StorageMain {
public:
    StorageMain();

    Re handle(Statement *stmt);

private:
    DIR *findBin();

    DIR *findDbDir();

    FILE *findDbFile(DIR *bin_dir);
};