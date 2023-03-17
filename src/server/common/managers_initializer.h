#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"
class ManagerInitializer{
    public:
    static ManagerInitializer &getInstance();
    void handle();
    void destroy();
    private:
};