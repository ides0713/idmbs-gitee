#include "base_main.h"

#include <assert.h>          // for assert

#include "../storage/txn.h"  // for Txn

void BaseMain::BaseSet(const BaseMain &base_main) {
    database_ = base_main.database_;
    txn_ = base_main.txn_;
    txn_multi_operation_ = base_main.txn_multi_operation_;
}
Txn *BaseMain::GetTxn() {
    if (txn_ == nullptr) {
        txn_ = new Txn();
        return txn_;
    }
    return txn_;
}
std::string StrMainType(MainType type) {
    switch (type) {
        case MainType::Base:
            return std::string{"Base"};
        case MainType::Start:
            return std::string{"Start"};
        case MainType::Parses:
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
