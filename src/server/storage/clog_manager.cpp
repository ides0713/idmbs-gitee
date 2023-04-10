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
                clog_flag_ = RedoError;
                debugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogInsertRecord;
                auto *clog_insert_record = static_cast<CLogInsertRecord *>(clog_record_);
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
                clog_flag_ = RedoError;
                debugPrint("CLogRecord:record is null,construct skipped\n");
            } else {
                clog_record_ = new CLogDeleteRecord;
                auto *clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
                clog_delete_record->rid = rec->getRecordId();
                strcpy(clog_delete_record->table_name, table_name);
                clog_record_->header.set(CLogManager::getNextLsn(sizeof(CLogDeleteRecord)),
                                         txn_id, clog_flag_, sizeof(CLogDeleteRecord));
            }
        }
            break;
        default: {
            debugPrint("CLogRecord:init failed,clog flag invalid\n");
            clog_flag_ = RedoError;
        }
            break;
    }
}

CLogRecord::CLogRecord(char *data) {
    auto *clog_record_header = reinterpret_cast<CLogRecordHeader *>(data);
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
            CLogInsertRecord *clog_insert_record = static_cast<CLogInsertRecord *>(clog_record_);
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
            auto *clog_delete_record = static_cast<CLogDeleteRecord *>(clog_record_);
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

Re CLogRecord::copyRecord(void *dest, int start_off, int copy_len) {
    return RecordEof;
}

int CLogRecord::cmpEq(CLogRecord *other) {
    return 0;
}

std::atomic<int32_t> CLogManager::global_lsn(0);

int32_t CLogManager::getNextLsn(int32_t rec_len) {
    int32_t res_lsn = CLogManager::global_lsn;
    CLogManager::global_lsn += rec_len;  // 当前不考虑溢出
    return res_lsn;
}

CLogManager::CLogManager(std::filesystem::path database_path) {

}
