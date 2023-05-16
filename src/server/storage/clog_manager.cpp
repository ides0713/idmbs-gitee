#include "clog_manager.h"
#include <bits/chrono.h>
#include <filesystem>
#include <new>
#include "../../common/common_defs.h"
#include "../common/persist_file_io_handler.h"
#include "../common/re.h"
const char *CLOG_FILE_NAME = "clog";
const size_t CLOG_INSERT_RECORD_NO_PTR_SIZE = sizeof(CLogInsertRecord) - sizeof(char *);
CLogRecord::CLogRecord(CLogType flag, int32_t txn_id, const char *table_name, int data_len, class Record *rec) {
    clog_flag_ = flag;
    clog_record_ = nullptr;
    switch (clog_flag_) {
        case CLogType::RedoMiniTxnBegin:
        case CLogType::RedoMiniTxnCommit: {
            clog_record_ = new CLogMiniTxnRecord;
            clog_record_->header.Set(CLogManager::GetNextLsn(sizeof(CLogMiniTxnRecord)), txn_id, clog_flag_,
                                     sizeof(CLogMiniTxnRecord));
        } break;
        case CLogType::RedoInsert: {
            if (rec == nullptr or rec->GetData() == nullptr) {
                clog_flag_ = CLogType::RedoError;
                DebugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogInsertRecord;
                auto clog_insert_record = static_cast<CLogInsertRecord *>(clog_record_);
                strcpy(clog_insert_record->table_name, table_name);
                clog_insert_record->rid = rec->GetRecordId();
                clog_insert_record->data_len = data_len;
                int clog_record_length = Align8(CLOG_INSERT_RECORD_NO_PTR_SIZE + data_len);
                clog_record_->header.Set(CLogManager::GetNextLsn(clog_record_length), txn_id, clog_flag_,
                                         clog_record_length);
                clog_insert_record->data = new char[clog_record_length - CLOG_INSERT_RECORD_NO_PTR_SIZE];
                memmove(clog_insert_record->data, rec->GetData(), data_len);
            }
        } break;
        case CLogType::RedoDelete: {
            if (rec == nullptr) {
                clog_flag_ = CLogType::RedoError;
                DebugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogDeleteRecord;
                auto clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
                clog_delete_record->rid = rec->GetRecordId();
                strcpy(clog_delete_record->table_name, table_name);
                clog_record_->header.Set(CLogManager::GetNextLsn(sizeof(CLogDeleteRecord)), txn_id, clog_flag_,
                                         sizeof(CLogDeleteRecord));
            }
        } break;
        default: {
            DebugPrint("CLogRecord:init failed,clog flag invalid\n");
            clog_flag_ = CLogType::RedoError;
        } break;
    }
}
CLogRecord::CLogRecord(char *data) {
    auto clog_record_header = reinterpret_cast<CLogRecordHeader *>(data);
    clog_flag_ = (CLogType) clog_record_header->type;
    switch (clog_flag_) {
        case CLogType::RedoMiniTxnBegin:
        case CLogType::RedoMiniTxnCommit: {
            clog_record_ = new CLogMiniTxnRecord;
            clog_record_->header = *clog_record_header;
        } break;
        case CLogType::RedoInsert: {
            clog_record_ = new CLogInsertRecord;
            auto clog_insert_record = static_cast<CLogInsertRecord *>(clog_record_);
            clog_record_->header = *clog_record_header;
            data += sizeof(CLogRecordHeader);
            strcpy(clog_insert_record->table_name, data);
            data += TABLE_NAME_MAX_LEN;
            clog_insert_record->rid = *reinterpret_cast<RecordId *>(data);
            data += sizeof(RecordId);
            clog_insert_record->data_len = *reinterpret_cast<int *>(data);
            data += sizeof(int);
            clog_insert_record->data = new char[clog_record_header->clog_record_len - CLOG_INSERT_RECORD_NO_PTR_SIZE];
            memmove(clog_insert_record->data, data, clog_insert_record->data_len);
        } break;
        case CLogType::RedoDelete: {
            clog_record_ = new CLogDeleteRecord;
            auto clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
            clog_record_->header = *clog_record_header;
            data += sizeof(CLogRecordHeader);
            strcpy(clog_delete_record->table_name, data);
            data += TABLE_NAME_MAX_LEN;
            clog_delete_record->rid = *reinterpret_cast<RecordId *>(data);
        } break;
        default: {
            DebugPrint("CLogRecord:init failed,clog flag invalid\n");
            clog_flag_ = CLogType::RedoError;
        } break;
    }
}
CLogRecord::~CLogRecord() {
    delete clog_record_;
}
Re CLogRecord::CopyRecordTo(void *dest, int start_off, int copy_len) {
    if (start_off + copy_len > GetCLogRecordLen()) {
        DebugPrint("copy record to dest failed,start off %d,copy len %d,total record len is %d\n", start_off, copy_len,
                   GetCLogRecordLen());
        return Re::GenericError;
    }
    switch (clog_flag_) {
        case CLogType::RedoInsert: {
            if (start_off > CLOG_INSERT_RECORD_NO_PTR_SIZE)
                memmove(dest,
                        static_cast<CLogInsertRecord *>(clog_record_)->data + start_off -
                                CLOG_INSERT_RECORD_NO_PTR_SIZE,
                        copy_len);
            else if (start_off + copy_len <= CLOG_INSERT_RECORD_NO_PTR_SIZE)
                memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, copy_len);
            else {
                int no_ptr_copy_len = CLOG_INSERT_RECORD_NO_PTR_SIZE - start_off;
                memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, no_ptr_copy_len);
                memmove(reinterpret_cast<char *>(dest) + no_ptr_copy_len,
                        static_cast<CLogInsertRecord *>(clog_record_)->data, copy_len - no_ptr_copy_len);
            }
        } break;
        default:
            memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, copy_len);
            break;
    }
    return Re::Success;
}
int CLogRecord::CmpEq(CLogRecord *other) {
    if (GetCLogType() == other->GetCLogType()) {
        switch (GetCLogType()) {
            case CLogType::RedoMiniTxnBegin:
            case CLogType::RedoMiniTxnCommit: {
                auto cr_1 = static_cast<CLogMiniTxnRecord *>(clog_record_),
                     cr_2 = static_cast<CLogMiniTxnRecord *>(other->GetCLogRecord());
                return *cr_1 == *cr_2;
            }
            case CLogType::RedoInsert: {
                auto cr_1 = static_cast<CLogInsertRecord *>(clog_record_),
                     cr_2 = static_cast<CLogInsertRecord *>(other->GetCLogRecord());
                return *cr_1 == *cr_2;
            }
            case CLogType::RedoDelete: {
                auto cr_1 = static_cast<CLogDeleteRecord *>(clog_record_),
                     cr_2 = static_cast<CLogDeleteRecord *>(other->GetCLogRecord());
                return *cr_1 == *cr_2;
            }
            default:
                return 0;
        }
    }
    return 0;
}
CLogBuffer::CLogBuffer() : current_block_offset_(0), write_block_offset_(0), write_offset_(0) {
    memset(buffer_, 0, CLOG_BUFFER_SIZE);
}
CLogBuffer::~CLogBuffer() {
}
Re CLogBuffer::AppendCLogRecord(CLogRecord *clog_record, int &offset) {
    if (clog_record == nullptr)
        return Re::GenericError;
    if (write_offset_ >= CLOG_BUFFER_SIZE)
        return Re::LogBufFull;
    int32_t clog_record_left_len = clog_record->GetCLogRecordLen() - offset;
    auto clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
    // current_block_offset start by 1*CLOG_BLOCK_SIZE because first block of clog file is head block,
    //  so we start from the second block in file but start by the first block in buffer
    if (write_offset_ == 0 and write_block_offset_ == 0) {
        memset(clog_block, 0, CLOG_BLOCK_SIZE);
        current_block_offset_ += CLOG_BLOCK_SIZE;
        clog_block->header.block_offset = current_block_offset_;
        write_offset_ = CLOG_BLOCK_HEADER_SIZE;
    }
    // current block is already full,time to get next block to write
    if (clog_block->header.block_data_len == CLOG_BLOCK_DATA_SIZE) {
        write_block_offset_ += CLOG_BLOCK_SIZE;
        current_block_offset_ += CLOG_BLOCK_SIZE;
        clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
        memset(clog_block, 0, CLOG_BLOCK_SIZE);
        clog_block->header.block_offset = current_block_offset_;
        write_offset_ += CLOG_BLOCK_HEADER_SIZE;
        return AppendCLogRecord(clog_record, offset);
    }
    // left data of the record can be stored in current block and no need to get a new block
    if (clog_record_left_len <= (CLOG_BLOCK_DATA_SIZE - clog_block->header.block_data_len)) {
        if (clog_block->header.block_data_len == 0)
            clog_block->header.first_record_offset = CLOG_BLOCK_HEADER_SIZE + (offset == 0 ? 0 : clog_record_left_len);
        clog_record->CopyRecordTo(&buffer_[write_offset_], offset, clog_record_left_len);
        write_offset_ += clog_record_left_len;
        clog_block->header.block_data_len += clog_record_left_len;
        offset += clog_record_left_len;
    } else {
        // left data can not be stored in a block,after stored block size byte data into buffer,we had to get new block
        if (clog_block->header.block_data_len == 0)                  // 当前为新block
            clog_block->header.first_record_offset = CLOG_BLOCK_SIZE;// no record starts in the block
        int32_t clog_block_data_left_len = CLOG_BLOCK_DATA_SIZE - clog_block->header.block_data_len;
        clog_record->CopyRecordTo(&(buffer_[write_offset_]), offset, clog_block_data_left_len);
        write_offset_ += clog_block_data_left_len;
        clog_block->header.block_data_len += clog_block_data_left_len;
        offset += clog_block_data_left_len;
        return AppendCLogRecord(clog_record, offset);
    }
    return Re::Success;
}
Re CLogBuffer::FlushBuffer(CLogFile *clog_file) {
    if (write_offset_ == CLOG_BUFFER_SIZE) {// 如果是buffer满触发的下刷
        auto clog_block = reinterpret_cast<CLogBlock *>(buffer_);
        clog_file->Write(clog_block->header.block_offset, CLOG_BUFFER_SIZE, buffer_);
        write_block_offset_ = 0, write_offset_ = 0;
        memset(buffer_, 0, CLOG_BUFFER_SIZE);
    } else {
        auto clog_block = reinterpret_cast<CLogBlock *>(buffer_);
        // write block offset+clog block size means?
        clog_file->Write(clog_block->header.block_offset, write_block_offset_ + CLOG_BLOCK_SIZE, buffer_);
        clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
        if (clog_block->header.block_data_len == CLOG_BLOCK_DATA_SIZE) {// 最后一个block已写满
            write_block_offset_ = 0, write_offset_ = 0;
            memset(buffer_, 0, CLOG_BUFFER_SIZE);
        } else if (write_block_offset_ != 0) {
            // move last block not full to the begin of buffer and reuse the buffer
            write_offset_ = clog_block->header.block_data_len + CLOG_BLOCK_HEADER_SIZE;
            memmove(buffer_, &buffer_[write_block_offset_], CLOG_BLOCK_SIZE);
            write_block_offset_ = 0;
            memset(&buffer_[CLOG_BLOCK_SIZE], 0, CLOG_BUFFER_SIZE - CLOG_BLOCK_SIZE);
        }
    }
    return Re::Success;
}
Re CLogBuffer::BlockCopyFrom(int32_t offset, CLogBlock *clog_block) {
    memmove(&buffer_[offset], reinterpret_cast<char *>(clog_block), CLOG_BLOCK_SIZE);
    return Re::Success;
}
CLogFile::CLogFile(const char *dir_path) {
    namespace fs = std::filesystem;
    clog_file_ = new PersistFileIoHandler();
    fs::path clog_file_path = fs::path(dir_path).append(CLOG_FILE_NAME);
    Re r = clog_file_->CreateFile(clog_file_path.c_str());
    if (r == Re::Success)
        UpdateCLogFileHeader(0);
    else if (r == Re::FileExist) {
        (void) clog_file_->OpenFile(clog_file_path.c_str());
        (void) clog_file_->ReadAt(0, CLOG_BLOCK_SIZE, reinterpret_cast<char *>(&clog_file_header_block_), nullptr);
    }
}
CLogFile::~CLogFile() {
    delete clog_file_;
}
Re CLogFile::UpdateCLogFileHeader(int32_t current_file_lsn) {
    clog_file_header_block_.header.file_lsn = current_file_lsn;
    clog_file_header_block_.header.file_real_offset = CLOG_FILE_HEADER_SIZE;
    return clog_file_->WriteAt(0, CLOG_BLOCK_SIZE, reinterpret_cast<char *>(&clog_file_header_block_), nullptr);
}
Re CLogFile::Append(int data_len, char *data) {
    return clog_file_->Append(data_len, data, nullptr);
}
Re CLogFile::Write(uint64_t offset, int data_len, char *data) {
    return clog_file_->WriteAt(offset, data_len, data, nullptr);
}
Re CLogFile::Recover(CLogMiniTxnManager *mini_txn_manager, CLogBuffer *clog_buffer) {
    // obviously,clog record can not be divided into three blocks but up to two blocks
    char redo_buffer[CLOG_REDO_BUFFER_SIZE];
    CLogRecordBuffer clog_record_buffer;
    memset(&clog_record_buffer, 0, sizeof(CLogRecordBuffer));
    CLogRecord *clog_record = nullptr;
    // skip the first block of the file(header block)
    uint64_t offset = CLOG_BLOCK_SIZE;
    size_t read_size = 0;
    clog_file_->ReadAt(offset, CLOG_REDO_BUFFER_SIZE, redo_buffer, &read_size);
    while (read_size != 0) {
        int32_t redo_buffer_offset = 0;
        while (redo_buffer_offset < CLOG_REDO_BUFFER_SIZE) {
            auto clog_block = reinterpret_cast<CLogBlock *>(&redo_buffer[redo_buffer_offset]);
            clog_buffer->SetCurrentBlockOffset(clog_block->header.block_offset);
            int16_t record_offset = CLOG_BLOCK_HEADER_SIZE;
            while (record_offset < CLOG_BLOCK_HEADER_SIZE + clog_block->header.block_data_len) {
                BlockRecover(clog_block, record_offset, &clog_record_buffer, clog_record);
                if (clog_record != nullptr) {
                    CLogManager::global_lsn = clog_record->GetLsn() + clog_record->GetCLogRecordLen();
                    mini_txn_manager->CLogRecordManage(clog_record);
                    clog_record = nullptr;
                }
            }
            if (clog_block->header.block_data_len < CLOG_BLOCK_DATA_SIZE) {// 最后一个block
                clog_buffer->BlockCopyFrom(0, clog_block);
                clog_buffer->SetWriteBlockOffset(0);
                clog_buffer->SetWriteOffset(clog_block->header.block_data_len + CLOG_BLOCK_HEADER_SIZE);
                if (clog_record_buffer.write_offset != 0) {
                    clog_record = new CLogRecord(reinterpret_cast<char *>(clog_record_buffer.buffer));
                    mini_txn_manager->CLogRecordManage(clog_record);
                }
                return Re::Success;
            }
            redo_buffer_offset += CLOG_BLOCK_SIZE;
        }
        offset += read_size;
        clog_file_->ReadAt(offset, CLOG_REDO_BUFFER_SIZE, redo_buffer, &read_size);
    }
    if (clog_record_buffer.write_offset != 0) {
        clog_record = new CLogRecord(reinterpret_cast<char *>(clog_record_buffer.buffer));
        mini_txn_manager->CLogRecordManage(clog_record);
    }
    return Re::Success;
}
Re CLogFile::BlockRecover(CLogBlock *block, int16_t &offset, CLogRecordBuffer *clog_record_buffer,
                          CLogRecord *&clog_record) {
    if (offset == CLOG_BLOCK_HEADER_SIZE and block->header.first_record_offset != CLOG_BLOCK_HEADER_SIZE) {
        // the record is not in only one block and in this case the part we recover is not the first part of the record
        // so we write from the header_end to first record begin(first_record_offset)
        int write_len = block->header.first_record_offset - CLOG_BLOCK_HEADER_SIZE;
        const char *write_target_ptr = reinterpret_cast<char *>(block) + static_cast<int>(offset);
        memmove(&clog_record_buffer->buffer[clog_record_buffer->write_offset], write_target_ptr, write_len);
        clog_record_buffer->write_offset += write_len;
        offset += write_len;
    } else {
        if (CLOG_BLOCK_SIZE - offset < sizeof(CLogRecordHeader)) {// 一定是跨block的第一部分
            // 此时无法确定log record的长度 开始写入clog_record_buffer
            int write_len = CLOG_BLOCK_SIZE - offset;
            const char *write_target_ptr = reinterpret_cast<char *>(block) + static_cast<int>(offset);
            memmove(&clog_record_buffer->buffer[clog_record_buffer->write_offset], write_target_ptr, write_len);
            clog_record_buffer->write_offset += write_len;
            offset = CLOG_BLOCK_SIZE;
        } else {
            if (clog_record_buffer->write_offset != 0) {
                clog_record = new CLogRecord(reinterpret_cast<char *>(clog_record_buffer->buffer));
                memset(clog_record_buffer, 0, sizeof(CLogRecordBuffer));
            } else {
                auto clog_record_header = reinterpret_cast<CLogRecordHeader *>(reinterpret_cast<char *>(block)) +
                                          static_cast<int>(offset);
                if (clog_record_header->clog_record_len <= CLOG_BLOCK_SIZE - offset) {
                    clog_record = new CLogRecord(reinterpret_cast<char *>(block) + static_cast<int>(offset));
                    offset += clog_record_header->clog_record_len;
                } else {
                    // 此时为跨block的第一部分 开始写入logrec_buf
                    int write_len = CLOG_BLOCK_SIZE - offset;
                    const char *write_target_ptr = reinterpret_cast<char *>(block) + static_cast<int>(offset);
                    memmove(&clog_record_buffer->buffer[clog_record_buffer->write_offset], write_target_ptr, write_len);
                    clog_record_buffer->write_offset += write_len;
                    offset = CLOG_BLOCK_SIZE;
                }
            }
        }
    }
    return Re::Success;
}
void CLogMiniTxnManager::CLogRecordManage(CLogRecord *clog_record) {
    switch (clog_record->GetCLogType()) {
        case CLogType::RedoMiniTxnCommit: {
            txn_committed[clog_record->GetTxnId()] = true;
            delete clog_record;
        } break;
        case CLogType::RedoMiniTxnBegin: {
            txn_committed.emplace(clog_record->GetTxnId(), false);
            delete clog_record;
        } break;
        default:
            clog_redo_list.push_back(clog_record);
            break;
    }
}
std::atomic<int32_t> CLogManager::global_lsn(0);
int32_t CLogManager::GetNextLsn(int32_t rec_len) {
    int32_t res_lsn = CLogManager::global_lsn;
    CLogManager::global_lsn += rec_len;// 当前不考虑溢出
    return res_lsn;
}
CLogManager::CLogManager(const char *dir_path) {
    clog_buffer_ = new CLogBuffer();
    clog_file_ = new CLogFile(dir_path);
    clog_mini_txn_manager_ = new CLogMiniTxnManager();
}
CLogManager::~CLogManager() {
    delete clog_buffer_;
}
Re CLogManager::MakeRecord(CLogType flag, int32_t txn_id, CLogRecord *&clog_record, const char *table_name,
                           int data_len, class Record *rec) {
    auto new_clog_record = new (std::nothrow) CLogRecord(flag, txn_id, table_name, data_len, rec);
    if (new_clog_record != nullptr)
        clog_record = new_clog_record;
    else {
        DebugPrint("CLogManager:new CLogRecord failed\n");
        return Re::NoMem;
    }
    return Re::Success;
}
Re CLogManager::AppendRecord(CLogRecord *clog_record) {
    int start_offset = 0;
    Re r = clog_buffer_->AppendCLogRecord(clog_record, start_offset);
    if (r == Re::LogBufFull or clog_record->GetCLogType() == CLogType::RedoMiniTxnCommit) {
        Sync();
        if (start_offset != clog_record->GetCLogRecordLen()) {// 当前日志记录还没写完
            clog_buffer_->AppendCLogRecord(clog_record, start_offset);
        }
    }
    delete clog_record;// NOTE: 单元测试需要注释该行
    return r;
}
Re CLogManager::Sync() {
    return clog_buffer_->FlushBuffer(clog_file_);
}
Re CLogManager::Recover() {
    clog_file_->Recover(clog_mini_txn_manager_, clog_buffer_);
    return Re::Success;
}
