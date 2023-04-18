#include "storage_main.h"
#include "storage_defs.h"
#include <sys/stat.h>
#include "../../common/common_defs.h"

Re StorageMain::handle()
{
    return Re::Success;
}

void StorageMain::response()
{
    printf("%s", execute_session_->getResponse().c_str());
}
