#pragma once
#include "re.h"
#include <vector>
#include <string>
#include <cstring>
class Session;
class DataBase;
class Txn;
enum MainType
{
    Start = 0,
    Parse,
    Resolve,
    Execute,
    Storage,
    Base
};
std::string strMainType(MainType type);
class BaseMain
{
public:
    BaseMain() : database_(nullptr), txn_(nullptr), txn_multi_operation_(false), type_(MainType::Base) {}
    BaseMain(DataBase *db, Txn *txn, bool txn_multi_operation) : database_(db), txn_(txn), txn_multi_operation_(txn_multi_operation), type_(MainType::Base) {}
    virtual Re init(BaseMain *last_main) = 0;
    virtual Re handle() = 0;
    virtual void clear() = 0;
    virtual void destroy() = 0;
    void baseSet(const BaseMain &base_main);
    DataBase *getDb() { return database_; }
    Txn *getTxn();
    MainType getType() { return type_; }
    bool getTMO() { return txn_multi_operation_; }
    void setDb(DataBase *database) { database_ = database; }
    void setTxn(Txn *txn) { txn_ = txn; }
    void setTmo(bool txn_multi_operation) { txn_multi_operation_ = txn_multi_operation; }
    void setType(MainType type) { type_ = type; }

protected:
    DataBase *database_;
    Txn *txn_;
    bool txn_multi_operation_;
    MainType type_;
};