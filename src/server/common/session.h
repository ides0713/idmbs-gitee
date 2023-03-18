#pragma once
#include "../storage/database.h"
#include "../storage/txn.h"
#include "../parse/parse_defs.h"
#include "../resolve/resolve_defs.h"
class Session
{
public:
    Session(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false)
        : database_(database), txn_(txn), txn_multi_operation_(txn_multi_operation) {}
    ~Session();
    DataBase *getDB() const { return database_; }
    void setDB(DataBase *database) { database_ = database; }
    Txn *getTXN() const { return txn_; }
    void setTXN(Txn *txn) { txn_ = txn; }
    bool getTMO() const { return txn_multi_operation_; }
    void setTMO(bool txn_multi_operation) { txn_multi_operation_ = txn_multi_operation; }

private:
    DataBase *database_;
    Txn *txn_;
    bool txn_multi_operation_;
};

class ParseSession : public Session
{
public:
    ParseSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false, Query *query = nullptr)
        : Session(database, txn, txn_multi_operation), query_(query) {}
    Query *getQuery() const { return query_; };
    void setQuery(Query *query) { query_ = query; }

private:
    Query *query_;
};

class ResolveSession : public Session
{
public:
    ResolveSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false, Statement *stmt = nullptr)
        : Session(database, txn, txn_multi_operation), stmt_(stmt) {}
    ResolveSession(Session* parse_session,Statement* stmt):Session(*parse_session),stmt_(stmt){}
    Statement *getSTMT() const { return stmt_; }
    void setSTMT(Statement *stmt) { stmt_ = stmt; }

private:
    Statement *stmt_;
};

class ExecuteSession : public Session
{
public:
    ExecuteSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false)
        : Session(database, txn, txn_multi_operation) {}

private:
};