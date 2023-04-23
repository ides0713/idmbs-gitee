#pragma once
#include <string>  // for string

#include "re.h"    // for Re

class DataBase;
class Txn;

enum MainType
{
    Start = 0,
    Parses,
    Resolve,
    Execute,
    Storage,
    Base
};
std::string StrMainType(MainType type);
class BaseMain
{
public:
    BaseMain() : database_(nullptr), txn_(nullptr), txn_multi_operation_(false), type_(MainType::Base) {}
    BaseMain(DataBase *db, Txn *txn, bool txn_multi_operation)
        : database_(db), txn_(txn), txn_multi_operation_(txn_multi_operation), type_(MainType::Base) {}
    virtual Re Init(BaseMain *last_main) = 0;
    virtual Re Handle() = 0;
    virtual void Clear() = 0;
    virtual void Destroy() = 0;
    void BaseSet(const BaseMain &base_main);
    DataBase *GetDb() { return database_; }
    Txn *GetTxn();
    MainType GetType() { return type_; }
    bool GetTmo() { return txn_multi_operation_; }
    void SetDb(DataBase *database) { database_ = database; }
    void SetTxn(Txn *txn) { txn_ = txn; }
    void SetTmo(bool txn_multi_operation) { txn_multi_operation_ = txn_multi_operation; }
    void SetType(MainType type) { type_ = type; }

protected:
    DataBase *database_;
    Txn *txn_;
    bool txn_multi_operation_;
    MainType type_;
};