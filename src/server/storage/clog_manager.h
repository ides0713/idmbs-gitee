#pragma once

#include <filesystem>
#include <atomic>
#include "../common/persist_file_io_handler.h"
#include "record.h"
#include "../common/re.h"

#define CLOG_FILE_SIZE 48 * 1024 * 1024
#define CLOG_BUFFER_SIZE 4 * 1024 * 1024
#define TABLE_NAME_MAX_LEN 20

enum CLogType {
    RedoError = 0,
    RedoMiniTxnBegin,
    RedoMiniTxnCommit,
    RedoInsert,
    RedoDelete,
    RedoUpdate
};

struct CLogRecordHeader {
public:
    int32_t lsn;//lsn stands for log sequence number
    int32_t txn_id;
    int type;
    int clog_record_len;
public:
    void set(int32_t p_lsn, int32_t p_txn_id, int p_type, int p_clog_record_len) {
        lsn = p_lsn;
        txn_id = p_txn_id;
        type = p_type;
        clog_record_len = p_clog_record_len;
    }

    bool operator==(const CLogRecordHeader &other) const {
        return lsn == other.lsn && txn_id == other.txn_id && type == other.type &&
               clog_record_len == other.clog_record_len;
    }
};

class CLogRecordBase {
public:
    CLogRecordHeader header;
public:
    CLogRecordBase() = default;

    virtual ~CLogRecordBase() = default;
};

class CLogInsertRecord : public CLogRecordBase {
public:
    char table_name[TABLE_NAME_MAX_LEN];
    RecordId rid;
    int data_len;
    char *data;
public:
    CLogInsertRecord() : data(nullptr) {}

    ~CLogInsertRecord() override { delete[] data; }

    bool operator==(const CLogInsertRecord &other) const {
        return header == other.header and strcmp(table_name, other.table_name) == 0 and rid == other.rid and
               data_len == other.data_len and (memcmp(data, other.data, data_len) == 0);
    }
};

class CLogDeleteRecord : public CLogRecordBase {
public:
    char table_name[TABLE_NAME_MAX_LEN];
    RecordId rid;
public:
    ~CLogDeleteRecord() override = default;

    bool operator==(const CLogDeleteRecord &other) const {
        return header == other.header and strcmp(table_name, other.table_name) == 0 && rid == other.rid;
    }
};

class CLogMiniTxnRecord : public CLogRecordBase {
public:
    ~CLogMiniTxnRecord() override = default;

    bool operator==(const CLogMiniTxnRecord &other) const {
        return header == other.header;
    }
};

class CLogError : public CLogRecordBase {
public:
    ~CLogError() override = default;

    void destroy() { delete this; }

    bool operator==(const CLogError &other) const {
        return header == other.header;
    }
};

class CLogRecord {
public:
    // TODO: lsn当前在内部分配
    // 对齐在内部处理
    CLogRecord(CLogType flag, int32_t txn_id, const char *table_name, int data_len,
               class Record *rec);

    // 从外存恢复log record
    explicit CLogRecord(char *data);

    ~CLogRecord();

    CLogType getCLogType() { return clog_flag_; }

    int32_t getTxnId() { return clog_record_->header.txn_id; }

    int32_t getCLogRecordLen() { return clog_record_->header.clog_record_len; }

    int32_t getLsn() { return clog_record_->header.lsn; }

    Re copyRecord(void *dest, int start_off, int copy_len);

    int cmpEq(CLogRecord *other);

    CLogRecordBase *getCLogRecord() { return clog_record_; }

private:
    CLogType clog_flag_;
    CLogRecordBase *clog_record_;
private:
    friend class DataBase;
};

class CLogManager {
public:
    explicit CLogManager(std::filesystem::path database_path);

public:
    static std::atomic<int32_t> global_lsn;
public:
    static int32_t getNextLsn(int32_t rec_len);

private:
};

