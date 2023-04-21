#include "base_main.h"
#include "../storage/txn.h"
void BaseMain::baseSet(const BaseMain &base_main)
{
    database_ = base_main.database_;
    txn_ = base_main.txn_;
    txn_multi_operation_ = base_main.txn_multi_operation_;
}

Txn *BaseMain::getTxn()
{
    if (txn_ == nullptr)
    {
        txn_ = new Txn();
        return txn_;
    }
    return txn_;
}

std::string strMainType(MainType type)
{
    switch (type)
    {
    case MainType::Base:
        return std::string{"Base"};
    case MainType::Start:
        return std::string{"Start"};
    case MainType::Parse:
        return std::string{"Parse"};
    case MainType::Resolve:
        return std::string{"Resolve"};
    case MainType::Execute:
        return std::string{"Execute"};
    case MainType::Storage:
        return std::string{"Storage"};
    default:
        assert(false);
    }
}
