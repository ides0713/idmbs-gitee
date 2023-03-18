#include "execute_main.h"
RE ExecuteMain::handle()
{
    return RE::SUCCESS;
}

Session *ExecuteMain::callBack()
{
    return execute_session_;
}
