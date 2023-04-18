#pragma once

#include <filesystem>
#include <atomic>
#include "../common/persist_file_io_handler.h"
#include "record.h"
#include "../common/re.h"

#define CLOG_FILE_SIZE (48 * 1024 * 1024)  // 48MB
#define CLOG_BUFFER_SIZE (4 * 1024 * 1024) // 4MB
#define TABLE_NAME_MAX_LEN 20

enum CLogType
{
    RedoError = 0,
    RedoMiniTxnBegin,
    RedoMiniTxnCommit,
    RedoInsert,
    RedoDelete,
    RedoUpdate
};

struct CLogRecordHeader
{
public:
    int32_t lsn; // lsn stands for log sequence number
    int32_t txn_id;
    int type;
    int clog_record_len;

public:
    void set(int32_t p_lsn, int32_t p_txn_id, int p_type, int p_clog_record_len)
    {
        lsn = p_lsn;
        txn_id = p_txn_id;
        type = p_type;
        clog_record_len = p_clog_record_len;
    }

    bool operator==(const CLogRecordHeader &other) const
    {
        return lsn == other.lsn && txn_id == other.txn_id && type == other.type &&
               clog_record_len == other.clog_record_len;
    }
};

class CLogRecordBase
{
public:
    CLogRecordHeader header;

public:
    CLogRecordBase() = default;

    virtual ~CLogRecordBase() = default;
};

class CLogInsertRecord : public CLogRecordBase
{
public:
    char table_name[TABLE_NAME_MAX_LEN];
    RecordId rid;
    int data_len;
    char *data;

public:
    CLogInsertRecord() : data(nullptr) {}

    ~CLogInsertRecord() override { delete[] data; }

    bool operator==(const CLogInsertRecord &other) const
    {
        return header == other.header and strcmp(table_name, other.table_name) == 0 and rid == other.rid and
               data_len == other.data_len and (memcmp(data, other.data, data_len) == 0);
    }
};

class CLogDeleteRecord : public CLogRecordBase
{
public:
    char table_name[TABLE_NAME_MAX_LEN];
    RecordId rid;

public:
    ~CLogDeleteRecord() override = default;

    bool operator==(const CLogDeleteRecord &other) const
    {
        return header == other.header and strcmp(table_name, other.table_name) == 0 && rid == other.rid;
    }
};

class CLogMiniTxnRecord : public CLogRecordBase
{
public:
    ~CLogMiniTxnRecord() override = default;

    bool operator==(const CLogMiniTxnRecord &other) const
    {
        return header == other.header;
    }
};

class CLogError : public CLogRecordBase
{
public:
    ~CLogError() override = default;

    void destroy() { delete this; }

    bool operator==(const CLogError &other) const
    {
        return header == other.header;
    }
};

class CLogRecord
{
public:
    // TODO: lsn当前在内部分配
    ///@brief construct a clog record with given params
    CLogRecord(CLogType flag, int32_t txn_id, const char *table_name, int data_len,
               class Record *rec);

    ///@brief construct from chars data,using for recover clog from disk
    explicit CLogRecord(char *data);

    ~CLogRecord();

    CLogType getCLogType() { return clog_flag_; }

    int32_t getTxnId() { return clog_record_->header.txn_id; }

    int32_t getCLogRecordLen() { return clog_record_->header.clog_record_len; }

    int32_t getLsn() { return clog_record_->header.lsn; }

    ///@brief copy data of record to dest(from (param:start_off) to (param:start_off+param:copy_len))
    Re copyRecordTo(void *dest, int start_off, int copy_len);

    int cmpEq(CLogRecord *other);

    CLogRecordBase *getCLogRecord() { return clog_record_; }

private:
    CLogType clog_flag_;
    CLogRecordBase *clog_record_;

private:
    friend class DataBase;
};

class CLogBlock;

class CLogFile;

class CLogBuffer
{
public:
    CLogBuffer();

    ~CLogBuffer();

    Re appendCLogRecord(CLogRecord *clog_record, int &offset);

    // 将buffer中的数据下刷到log_file
    Re flushBuffer(CLogFile *clog_file);

    void setCurrentBlockOffset(const int32_t block_offset) { current_block_offset_ = block_offset; }

    void setWriteBlockOffset(const int32_t write_block_offset) { write_block_offset_ = write_block_offset; };

    void setWriteOffset(const int32_t write_offset) { write_offset_ = write_offset; };

    Re blockCopyFrom(int32_t offset, CLogBlock *clog_block);

private:
    int32_t current_block_offset_; // offset of the block (of the first byte of the block)
    int32_t write_block_offset_;
    int32_t write_offset_;
    char buffer_[CLOG_BUFFER_SIZE]; // 4MB
};

struct CLogBlockHeader;
struct CLogFileHeader;

#define CLOG_FILE_HEADER_SIZE sizeof(CLogFileHeader)                    // 8B
#define CLOG_BLOCK_SIZE (1 << 9)                                        // 512B
#define CLOG_BLOCK_HEADER_SIZE sizeof(CLogBlockHeader)                  // 8B
#define CLOG_BLOCK_DATA_SIZE (CLOG_BLOCK_SIZE - CLOG_BLOCK_HEADER_SIZE) // 504B (512B-8B)
#define CLOG_REDO_BUFFER_SIZE (8 * CLOG_BLOCK_SIZE)                     // 4KB

struct CLogRecordBuffer
{
public:
    int32_t write_offset;
    // TODO: 当前假定log record大小不会超过CLOG_REDO_BUFFER_SIZE
    char buffer[CLOG_REDO_BUFFER_SIZE];
};

struct CLogFileHeader
{
public:
    int32_t file_real_offset;
    // TODO: 用于文件组，当前没用
    int32_t file_lsn;
};

struct CLogFileHeaderBlock
{
public:
    CLogFileHeader header;
    char pad[CLOG_BLOCK_SIZE - CLOG_FILE_HEADER_SIZE];
};

struct CLogBlockHeader
{
public:
    int32_t block_offset;        // offset of the block in clock buffer
    int16_t block_data_len;      // len of current data stored in the block
    int16_t first_record_offset; // offset of first record start in the block(first record in the block ends at offset)
};

///@brief structure used to explain the buffer
struct CLogBlock
{
public:
    CLogBlockHeader header;
    char data[CLOG_BLOCK_DATA_SIZE];
};

class CLogMiniTxnManager;

class CLogFile
{
public:
    CLogFile(const char *dir_path);

    ~CLogFile();

    Re updateCLogFileHeader(int32_t current_file_lsn);

    Re append(int data_len, char *data);

    Re write(uint64_t offset, int data_len, char *data);

    ///@brief r
    Re recover(CLogMiniTxnManager *mini_txn_manager, CLogBuffer *clog_buffer);

    ///@brief recover a clog_record from block at param:offset
    ///@param offset offset in the block from the block header
    ///@param clog_record_buffer used to store the temp result of clog record
    ///@param clog_record result passed by reference of the ptr
    ///@NOTE all of the operation happen in one block;take into
    Re blockRecover(CLogBlock *block, int16_t &offset, CLogRecordBuffer *clog_record_buffer, CLogRecord *&clog_record);

private:
    CLogFileHeaderBlock clog_file_header_block_;
    PersistFileIoHandler *clog_file_;
};

// TODO: 当前简单管理mtr
struct CLogMiniTxnManager
{
public:
    std::list<CLogRecord *> clog_redo_list;
    std::unordered_map<int32_t, bool> txn_committed; // <txn_id, committed>
public:
    void cLogRecordManage(CLogRecord *clog_record);
};

class CLogManager
{
public:
    explicit CLogManager(const char *dir_path);

    ~CLogManager();

    Re makeRecord(CLogType flag, int32_t txn_id, CLogRecord *&clog_record, const char *table_name = nullptr,
                  int data_len = 0, class Record *rec = nullptr);

    // 追加写到log_buffer
    Re appendRecord(CLogRecord *clog_record);

    // TODO: 优化回放过程，对同一位置的修改可以用哈希聚合
    Re recover();

    CLogMiniTxnManager *getMiniTxnManager() { return clog_mini_txn_manager_; }

public:
    static std::atomic<int32_t> global_lsn;

public:
    static int32_t getNextLsn(int32_t rec_len);

private:
    CLogBuffer *clog_buffer_;
    CLogFile *clog_file_;
    CLogMiniTxnManager *clog_mini_txn_manager_;

private:
    // 通常不需要在外部调用
    Re sync();
};
