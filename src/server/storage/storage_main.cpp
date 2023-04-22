#include "storage_main.h"
#include "../../common/common_defs.h"
#include "storage_defs.h"
#include <sys/stat.h>
Re StorageMain::Init(BaseMain *last_main) {
    BaseSet(*last_main);
    return Re::Success;
}
Re StorageMain::Handle() {
    return Re::Success;
}
void StorageMain::Clear() {
    return;
}
void StorageMain::Destroy() {
    return;
}
