#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"

class GlobalManagersInitializer {
public:
    static GlobalManagersInitializer &getInstance();

    void handle();

    void destroy();

private:
};