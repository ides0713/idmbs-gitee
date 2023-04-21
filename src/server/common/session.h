#pragma once

#include "../storage/database.h"
#include "../storage/txn.h"
#include "../parse/parse_defs.h"
#include "../resolve/resolve_defs.h"

// class Session
// {
// public:
//     explicit Session(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false)
//         : database_(database), txn_(txn), txn_multi_operation_(txn_multi_operation)
//     {}

//     ~Session() = default;

//     [[nodiscard]] DataBase *getDb() const { return database_; }

//     void setDb(DataBase *database) { database_ = database; }

//     [[nodiscard]] Txn *getTxn();

//     void setTxn(Txn *txn) { txn_ = txn; }

//     [[nodiscard]] bool getTmo() const { return txn_multi_operation_; }

//     void setTmo(bool txn_multi_operation) { txn_multi_operation_ = txn_multi_operation; }

//     void setResponse(const char *response) { response_ = std::string(response); }

//     void setResponse(std::string &&response) { response_ = std::move(response); }
//     const std::string getResponse() { return response_; }

// private:
//     DataBase *database_;
//     Txn *txn_;
//     bool txn_multi_operation_;
//     std::string response_;
// };

// class ParseSession : public Session
// {
// public:
//     explicit ParseSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false,
//                           Query *query = nullptr)
//         : Session(database, txn, txn_multi_operation), query_(query) {}

//     [[nodiscard]] Query *getQuery() const { return query_; };

//     void setQuery(Query *query) { query_ = query; }

// private:
//     Query *query_;
// };

// class ResolveSession : public Session
// {
// public:
//     explicit ResolveSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false,
//                             Statement *stmt = nullptr)
//         : Session(database, txn, txn_multi_operation), stmt_(stmt) {}

//     ResolveSession(Session *parse_session, Statement *stmt) : Session(*parse_session), stmt_(stmt) {}

//     [[nodiscard]] Statement *getStmt() const { return stmt_; }

//     void setStmt(Statement *stmt) { stmt_ = stmt; }

// private:
//     Statement *stmt_;
// };

// class ExecuteSession : public Session
// {
// public:
//     explicit ExecuteSession(DataBase *database = nullptr, Txn *txn = nullptr, bool txn_multi_operation = false)
//         : Session(database, txn, txn_multi_operation) {}

//     explicit ExecuteSession(Session *resolve_session) : Session(*resolve_session), stmt_(static_cast<ResolveSession *>(resolve_session)->getStmt()) {}

// private:
//     Statement *stmt_;
// };