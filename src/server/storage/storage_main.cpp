#include "storage_main.h"

#include "/home/ubuntu/idbms/src/server/common/re.h"  // for Re, Success

class BaseMain;

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
