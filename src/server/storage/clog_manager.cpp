#include "clog_manager.h"

const char *CLOG_FILE_NAME = "clog";
const size_t CLOG_INSERT_RECORD_NO_PTR_SIZE = sizeof(CLogInsertRecord) - sizeof(char *);

CLogRecord::CLogRecord(CLogType flag, int32_t txn_id, const char *table_name, int data_len, class Record *rec) {
    clog_flag_ = flag;
    clog_record_ = nullptr;
    switch (clog_flag_) {
        case CLogType::RedoMiniTxnBegin:
        case CLogType::RedoMiniTxnCommit: {
            clog_record_ = new CLogMiniTxnRecord;
            clog_record_->header.set(CLogManager::getNextLsn(sizeof(CLogMiniTxnRecord)),
                                     txn_id, clog_flag_, sizeof(CLogMiniTxnRecord));
        }
            break;
        case CLogType::RedoInsert: {
            if (rec == nullptr or rec->getData() == nullptr) {
                clog_flag_ = CLogType::RedoError;
                debugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogInsertRecord;
                auto clog_insert_record = static_cast<CLogInsertRecord *>(clog_record_);
                strcpy(clog_insert_record->table_name, table_name);
                clog_insert_record->rid = rec->getRecordId();
                clog_insert_record->data_len = data_len;
                int clog_record_length = align8(CLOG_INSERT_RECORD_NO_PTR_SIZE + data_len);
                clog_record_->header.set(CLogManager::getNextLsn(clog_record_length),
                                         txn_id, clog_flag_, clog_record_length);
                clog_insert_record->data = new char[clog_record_length - CLOG_INSERT_RECORD_NO_PTR_SIZE];
                memmove(clog_insert_record->data, rec->getData(), data_len);
            }
        }
            break;
        case CLogType::RedoDelete: {
            if (rec == nullptr) {
                clog_flag_ = CLogType::RedoError;
                debugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogDeleteRecord;
                auto clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
                clog_delete_record->rid = rec->getRecordId();
                strcpy(clog_delete_record->table_name, table_name);
                clog_record_->header.set(CLogManager::getNextLsn(sizeof(CLogDeleteRecord)),
                                         txn_id, clog_flag_, sizeof(CLogDeleteRecord));
            }
        }
            break;
        default: {
            debugPrint("CLogRecord:init failed,clog flag invalid\n");
            clog_flag_ = CLogType::RedoError;
        }
            break;
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
        }
            break;
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
        }
            break;
        case CLogType::RedoDelete: {
            clog_record_ = new CLogDeleteRecord;
            auto clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
            clog_record_->header = *clog_record_header;
            data += sizeof(CLogRecordHeader);
            strcpy(clog_delete_record->table_name, data);
            data += TABLE_NAME_MAX_LEN;
            clog_delete_record->rid = *reinterpret_cast<RecordId *>(data);
        }
            break;
        default: {
            debugPrint("CLogRecord:init failed,clog flag invalid\n");
            clog_flag_ = CLogType::RedoError;
        }
            break;
    }
}

CLogRecord::~CLogRecord() {
    delete clog_record_;
}

Re CLogRecord::copyRecordTo(void *dest, int start_off, int copy_len) {
    if (start_off + copy_len > getCLogRecordLen()) {
        debugPrint("copy record to dest failed,start off %d,copy len %d,total record len is %d\n",
                   start_off, copy_len, getCLogRecordLen());
        return Re::GenericError;
    }
    switch (clog_flag_) {
        case CLogType::RedoInsert: {
            if (start_off > CLOG_INSERT_RECORD_NO_PTR_SIZE)
                memmove(dest, static_cast<CLogInsertRecord *>(clog_record_)->data
                              + start_off - CLOG_INSERT_RECORD_NO_PTR_SIZE, copy_len);
            else if (start_off + copy_len <= CLOG_INSERT_RECORD_NO_PTR_SIZE)
                memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, copy_len);
            else {
                int no_ptr_copy_len = CLOG_INSERT_RECORD_NO_PTR_SIZE - start_off;
                memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, no_ptr_copy_len);
                memmove(reinterpret_cast<char *>(dest) + no_ptr_copy_len,
                        static_cast<CLogInsertRecord *>(clog_record_)->data,
                        copy_len - no_ptr_copy_len);
            }
        }
            break;
        default:
            memmove(dest, reinterpret_cast<char *>(clog_record_) + start_off, copy_len);
            break;
    }
    return Re::Success;
}

int CLogRecord::cmpEq(CLogRecord *other) {
    if (getCLogType() == other->getCLogType()) {
        switch (getCLogType()) {
            case CLogType::RedoMiniTxnBegin:
            case CLogType::RedoMiniTxnCommit: {
                auto cr_1 = static_cast<CLogMiniTxnRecord *>(clog_record_), cr_2 = static_cast<CLogMiniTxnRecord *>(other->getCLogRecord());
                return *cr_1 == *cr_2;
            }
            case CLogType::RedoInsert: {
                auto cr_1 = static_cast<CLogInsertRecord *>(clog_record_), cr_2 = static_cast<CLogInsertRecord *>(other->getCLogRecord());
                return *cr_1 == *cr_2;
            }
            case CLogType::RedoDelete: {
                auto cr_1 = static_cast<CLogDeleteRecord *>(clog_record_), cr_2 = static_cast<CLogDeleteRecord *>(other->getCLogRecord());
                return *cr_1 == *cr_2;
            }
            default:
                return 0;
        }
    }
    return 0;
}

CLogBuffer::CLogBuffer() : current_block_end_offset_(0), write_block_offset_(0), write_offset_(0) {
    memset(buffer_, 0, CLOG_BUFFER_SIZE);
}

CLogBuffer::~CLogBuffer() {}

Re CLogBuffer::appendCLogRecord(CLogRecord *clog_record, int &offset) {
    if (clog_record == nullptr)
        return Re::GenericError;
    if (write_offset_ >= CLOG_BUFFER_SIZE)
        return Re::LogBufFull;
    int32_t clog_record_left_len = clog_record->getCLogRecordLen() - offset;
    auto clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
    //if it is the first block of the
    if (write_offset_ == 0 and write_block_offset_ == 0) {
        memset(clog_block, 0, CLOG_BLOCK_SIZE);
        current_block_end_offset_ += CLOG_BLOCK_SIZE;
        clog_block->header.block_end_offset = current_block_end_offset_;
        write_offset_ = CLOG_BLOCK_HEADER_SIZE;
    }
    //current block is already full,time to get next block to write
    if (clog_block->header.block_data_len == CLOG_BLOCK_DATA_SIZE) {
        write_block_offset_ += CLOG_BLOCK_SIZE;
        current_block_end_offset_ += CLOG_BLOCK_SIZE;
        clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
        memset(clog_block, 0, CLOG_BLOCK_SIZE);
        clog_block->header.block_end_offset = current_block_end_offset_;
        write_offset_ += CLOG_BLOCK_HEADER_SIZE;
        return appendCLogRecord(clog_record, offset);
    }
    //left data of the record can be stored in current block and no need to get a new block
    if (clog_record_left_len <= (CLOG_BLOCK_DATA_SIZE - clog_block->header.block_data_len)) {
        if (clog_block->header.block_data_len == 0)
            clog_block->header.first_record_offset =
                    CLOG_BLOCK_HEADER_SIZE + (offset == 0 ? 0 : clog_record_left_len);
        clog_record->copyRecordTo(&buffer_[write_offset_], offset, clog_record_left_len);
        write_offset_ += clog_record_left_len;
        clog_block->header.block_data_len += clog_record_left_len;
        offset += clog_record_left_len;
    } else {
        //left data can not be stored in a block,after stored block size byte data into buffer,we had to get new block
        if (clog_block->header.block_data_len == 0) //当前为新block
            clog_block->header.first_record_offset = CLOG_BLOCK_SIZE;//no record starts in the block
        int32_t clog_block_data_left_len = CLOG_BLOCK_DATA_SIZE - clog_block->header.block_data_len;
        clog_record->copyRecordTo(&(buffer_[write_offset_]), offset, clog_block_data_left_len);
        write_offset_ += clog_block_data_left_len;
        clog_block->header.block_data_len += clog_block_data_left_len;
        offset += clog_block_data_left_len;
        return appendCLogRecord(clog_record, offset);
    }
    return Re::Success;
}

Re CLogBuffer::flushBuffer(CLogFile *clog_file) {
    if (write_offset_ == CLOG_BUFFER_SIZE) {  //如果是buffer满触发的下刷
        auto clog_block = reinterpret_cast<CLogBlock *>(buffer_);
        clog_file->write(clog_block->header.block_end_offset, CLOG_BUFFER_SIZE, buffer_);
        write_block_offset_ = 0, write_offset_ = 0;
        memset(buffer_, 0, CLOG_BUFFER_SIZE);
    } else {
        auto clog_block = reinterpret_cast<CLogBlock *>(buffer_);
        //write block offset+clog block size means?
        clog_file->write(clog_block->header.block_end_offset, write_block_offset_ + CLOG_BLOCK_SIZE, buffer_);
        clog_block = reinterpret_cast<CLogBlock *>(&buffer_[write_block_offset_]);
        if (clog_block->header.block_data_len == CLOG_BLOCK_DATA_SIZE) {  // 最后一个block已写满
            write_block_offset_ = 0, write_offset_ = 0;
            memset(buffer_, 0, CLOG_BUFFER_SIZE);
        } else if (write_block_offset_ != 0) {
            // 将最后一个未写满的block迁移到buffer起始位置 ???
            write_offset_ = clog_block->header.block_data_len + CLOG_BLOCK_HEADER_SIZE;
            memmove(buffer_, &buffer_[write_block_offset_], CLOG_BLOCK_SIZE);
            write_block_offset_ = 0;
            memset(&buffer_[CLOG_BLOCK_SIZE], 0, CLOG_BUFFER_SIZE - CLOG_BLOCK_SIZE);
        }
    }
    return Re::Success;
}

Re CLogBuffer::blockCopyFrom(int32_t offset, CLogBlock *log_block) {
    memmove(&buffer_[offset], reinterpret_cast<char *>(log_block), CLOG_BLOCK_SIZE);
    return Re::Success;
}

CLogFile::CLogFile(const char *dir_path) {
    namespace fs = std::filesystem;
    clog_file_ = new PersistFileIoHandler();
    fs::path clog_file_path = fs::path(dir_path).append(CLOG_FILE_NAME);
    Re r = clog_file_->createFile(clog_file_path.c_str());
    if (r == Re::Success)
        updateCLogFileHeader(0);
    else if (r == Re::FileExist) {
        (void) clog_file_->openFile(clog_file_path.c_str());
        (void) clog_file_->readAt(0, CLOG_BLOCK_SIZE, reinterpret_cast<char *>(&clog_file_header_block_), nullptr);
    }
}

CLogFile::~CLogFile() {
    delete clog_file_;
}

Re CLogFile::updateCLogFileHeader(int32_t current_file_lsn) {
    clog_file_header_block_.header.file_lsn = current_file_lsn;
    clog_file_header_block_.header.file_real_offset = CLOG_FILE_HEADER_SIZE;
    return clog_file_->writeAt(0, CLOG_BLOCK_SIZE, reinterpret_cast<char *>(&clog_file_header_block_), nullptr);
}

Re CLogFile::append(int data_len, char *data) {
    return clog_file_->append(data_len, data, nullptr);
}

Re CLogFile::write(uint64_t offset, int data_len, char *data) {
    return clog_file_->writeAt(offset, data_len, data, nullptr);
}

Re CLogFile::recover(CLogMiniTxnManager *mini_txn_manager, CLogBuffer *clog_buffer) {
    char redo_buffer[CLOG_REDO_BUFFER_SIZE];
    CLogRecordBuffer clog_record_buffer;
    memset(&clog_record_buffer, 0, sizeof(CLogRecordBuffer));
    CLogRecord *clog_record = nullptr;
    uint64_t offset = CLOG_BLOCK_SIZE;
    size_t read_size = 0;
    clog_file_->readAt(offset, CLOG_REDO_BUFFER_SIZE, redo_buffer, &read_size);
    while (read_size == 0) {
        int32_t buffer_offset = 0;
        while (buffer_offset < CLOG_REDO_BUFFER_SIZE) {
            auto clog_block = reinterpret_cast<CLogBlock *>(&redo_buffer[buffer_offset]);
            clog_buffer->setCurrentBlockOffset(clog_block->header.block_end_offset);
            int16_t record_offset = CLOG_BLOCK_HEADER_SIZE;
            while (record_offset < CLOG_BLOCK_HEADER_SIZE + clog_block->header.block_data_len) {
                blockRecover(clog_block, record_offset, &clog_record_buffer, clog_record);
                if (clog_record != nullptr) {
                    CLogManager::global_lsn = clog_record->getLsn() + clog_record->getCLogRecordLen();
                    mini_txn_manager->cLogRecordManage(clog_record);
                    clog_record = nullptr;
                }
            }
            if (clog_block->header.block_data_len < CLOG_BLOCK_DATA_SIZE) {  //最后一个block
                clog_buffer->blockCopyFrom(0, clog_block);
                clog_buffer->setWriteBlockOffset(0);
                clog_buffer->setWriteBlockOffset(clog_block->header.block_data_len + CLOG_BLOCK_HEADER_SIZE);
                if (clog_record_buffer.write_offset != 0) {
                    clog_record = new CLogRecord(reinterpret_cast<char *> (clog_record_buffer.buffer));
                    mini_txn_manager->cLogRecordManage(clog_record);
                }
                return Re::Success;
            }
            buffer_offset += CLOG_BLOCK_SIZE;
        }
        offset += read_size;
        clog_file_->readAt(offset, CLOG_REDO_BUFFER_SIZE, redo_buffer, &read_size);
    }
    if (clog_record_buffer.write_offset != 0) {
        clog_record = new CLogRecord(reinterpret_cast<char *> (clog_record_buffer.buffer));
        mini_txn_manager->cLogRecordManage(clog_record);
    }
    return Re::Success;
}

Re CLogFile::blockRecover(CLogBlock *block, int16_t &offset, CLogRecordBuffer *clog_record_buffer,
                          CLogRecord *&clog_record) {
    return RecordEof;
}

void CLogMiniTxnManager::cLogRecordManage(CLogRecord *clog_record) {
    switch (clog_record->getCLogType()) {
        case CLogType::RedoMiniTxnCommit: {
            txn_committed[clog_record->getTxnId()] = true;
            delete clog_record;
        }
            break;
        case CLogType::RedoMiniTxnBegin: {
            txn_committed.emplace(clog_record->getTxnId(), false);
            delete clog_record;
        }
            break;
        default:
            clog_redo_list.push_back(clog_record);
            break;
    }
}


std::atomic<int32_t> CLogManager::global_lsn(0);

int32_t CLogManager::getNextLsn(int32_t rec_len) {
    int32_t res_lsn = CLogManager::global_lsn;
    CLogManager::global_lsn += rec_len;  // 当前不考虑溢出
    return res_lsn;
}

CLogManager::CLogManager(std::filesystem::path database_path) {

}

