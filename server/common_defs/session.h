#pragma once
#include "../storage/database.h"
#include "../storage/txn.h"

class Session
{
    public:
    Session();
    ~Session();
    DataBase* getDB() const{return db_;}
    Txn* getTXN() const {return txn_;}
    
    private:
    DataBase* db_;
    Txn* txn_;
    bool txn_multi_operation_;
};
