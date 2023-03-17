#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"
class GlobalManagerInitializer{
    public:
    static GlobalManagerInitializer &getInstance();
    void handle();
    void destroy();
    private:
};