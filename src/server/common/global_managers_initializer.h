#include "server_defs.h"
#include "../storage/database.h"
#include "../storage/buffer_pool.h"

class GlobalManagersManager {
public:
    static GlobalManagersManager &getInstance();

    void handle();

    void destroy();

private:
};