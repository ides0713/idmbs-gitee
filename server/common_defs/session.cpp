#include "session.h"

Session::Session()
{
    db_=nullptr,txn_=nullptr;
    txn_multi_operation_=false;
}
