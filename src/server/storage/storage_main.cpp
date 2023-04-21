#include "storage_main.h"
#include "storage_defs.h"
#include <sys/stat.h>
#include "../../common/common_defs.h"

Re StorageMain::init(BaseMain *last_main)
{
    baseSet(*last_main);
    return Re::Success;
}

Re StorageMain::handle()
{
    return Re::Success;
}

void StorageMain::clear()
{
    return;
}

void StorageMain::destroy()
{
    return;
}
