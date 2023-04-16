#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <cstdint>
#include <sstream>
#include "bitmap.h"
#include "buffer_pool.h"
#include "../common/re.h"

int align8(int size);

int pageHeaderSize();

int pageRecordCapacity(int page_data_size, int record_size);

int pageBitmapSize(int record_capacity);

int pageHeaderSize(int record_capacity);

struct RecordId {
public:
    int32_t page_id, slot_id;
public:
    RecordId() = default;

    RecordId(const int32_t &pid, const int32_t &sid) : page_id(pid), slot_id(sid) {}

    [[nodiscard]] std::string toString() const;

    bool operator==(const RecordId &other) const { return page_id == other.page_id && slot_id == other.slot_id; }

    bool operator!=(const RecordId &other) const { return !(*this == other); }

public:
    static int compare(const RecordId *rid_1, const RecordId *rid_2);

    static RecordId *min();

    static RecordId *max();
};

class Record {
public:
    Record() : data_(nullptr) {}

    ///@brief set record_id and data of record
    void init(const RecordId &record_id, char *data);

    void setRecordId(const RecordId &record_id) { record_id_ = record_id; }

    void setRecordId(const int32_t page_id, const int32_t slot_id) {
        this->record_id_.page_id = page_id;
        this->record_id_.slot_id = slot_id;
    }

    void setData(char *data) { data_ = data; };

    [[nodiscard]] RecordId &getRecordId() { return record_id_; }

    [[nodiscard]] char *getData(){ return data_; }

private:
    RecordId record_id_;
    char *data_;
};

class ConditionFilter;

///@brief there are some meta params in page header
///@NOTE page header is part of page's data
///@n actually,page header is composed of 2 parts--struct PageHeader and bitmap of the page
struct PageHeader {
    int32_t record_num;           // 当前页面记录的个数
    int32_t record_capacity;      // 最大记录个数
    int32_t record_real_size;     // 每条记录的实际大小
    int32_t record_size;          // 每条记录占用实际空间大小(可能对齐)
    int32_t first_record_offset;  // 第一条记录的偏移量
};

class RecordPageHandler;

///@brief iterator of records in a page
class RecordPageIterator {
public:
    RecordPageIterator() : record_page_handler_(nullptr), page_id_(BP_INVALID_PAGE_NUM), next_slot_id_(0) {}

    void init(RecordPageHandler &record_page_handler);

    bool hasNext();

    Re next(class Record &record);

    [[nodiscard]] bool isValid() const { return record_page_handler_ != nullptr; }

private:
    RecordPageHandler *record_page_handler_;
    int32_t page_id_;//current page's page id
    int32_t next_slot_id_;//next slot id to return by func:next
    BitMap bitmap_;
};

class RecordFileHandler;

class RecordPageHandler {
public:
    RecordPageHandler() : disk_buffer_pool_(nullptr), frame_(nullptr), page_header_(nullptr), bitmap_(nullptr) {}

    ///@brief cleanUp() to destruct
    Re destroy();

    ///@brief init
    Re init(DiskBufferPool &buffer_pool, int32_t page_id);

    ///@brief recover init
    Re recoverInit(DiskBufferPool &buffer_pool, int32_t page_id);

    ///@brief set target page to empty page(includes page header and page data) in buffer and flush it;
    Re initEmptyPage(DiskBufferPool &buffer_pool, int32_t page_id, int record_size);

    ///@brief get page id of page in frame
    [[nodiscard]] int32_t getPageId() const;

    ///@brief return whether or not current page is full
    [[nodiscard]] bool isFull() const;

    ///@brief get target record's data
    char *getRecordData(int32_t slot_id);

    ///@brief insert record to next usable slot
    Re insertRecord(const char *data, RecordId *rid);

    ///@brief change param:rec's corresponding slot id with its data
    ///@NOTE use rec as a container of slot id and data
    Re updateRecord(class Record *rec);

    ///@brief delete record from the page(set corresponding bit of bitmap to 0)
    ///@NOTE delete record will not set space of record to 0 or so on,but just set bit to 0;
    ///@n when there is no record in the page,it will be disposed
    Re deleteRecord(const RecordId *rid);

    ///@brief get record by record id(param:rid) and pass it by param:rec
    Re getRecord(const RecordId *rid, class Record *rec);

    ///@brief recover insert record
    Re recoverInsertRecord(const char *data, RecordId *rid);

    template<class RecordUpdater>
    Re updateRecordInPlace(const RecordId *rid, RecordUpdater updater);

private:
    DiskBufferPool *disk_buffer_pool_;
    Frame *frame_;
    PageHeader *page_header_;
    char *bitmap_;

private:
    friend class RecordPageIterator;
};

///@brief use record page handler
class RecordFileHandler {
public:
    RecordFileHandler() = default;

    ///@brief link buffer of file with record file handler and add all free pages to free_pages_(initFreePages)
    Re init(DiskBufferPool *buffer_pool);

    ///@brief set buffer_pool_ to nullptr,unlink the record file handler with buffer of file
    void destroy();

    ///@brief init a record page handler and update record
    ///@NOTE rec->rid is record id of record we're going to update,rec->data is new data of record
    Re updateRecord(class Record *rec);

    ///@brief delete record with given record id
    Re deleteRecord(const RecordId *rid);

    ///@brief insert record to free page
    Re insertRecord(const char *data, int record_size, RecordId *rid);

    ///@brief recover insert record
    Re recoverInsertRecord(const char *data, int record_size, RecordId *rid);

    ///@brief get record with given record id and pass it by param:rec
    Re getRecord(const RecordId *rid, class Record *rec);

    template<class RecordUpdater>
    Re updateRecordInPlace(const RecordId *rid, RecordUpdater updater);

private:
    DiskBufferPool *disk_buffer_pool_ = nullptr;
    std::unordered_set<int32_t> free_pages_; // 没有填充满的页面集合

private:
    ///@brief add all pages that is not full to free_pages_
    Re initFreePages();
};

class RecordFileScanner {
public:
    RecordFileScanner() : disk_buffer_pool_(nullptr), condition_filter_(nullptr) {}

    ///@brief open a file and scan it,return all records fit the condition filter
    Re init(DiskBufferPool &buffer_pool, ConditionFilter *condition_filter);

    ///@brief unlink buffer of file and condition filter with iterator
    Re destroy();

    bool hasNext();

    Re next(class Record &record);

private:
    DiskBufferPool *disk_buffer_pool_;
    BufferPoolIterator buffer_pool_iterator_;
    ConditionFilter *condition_filter_;
    RecordPageHandler record_page_handler_;
    RecordPageIterator record_page_iterator_;

    class Record next_record_;

private:
    ///@brief fetch record
    ///@NOTE if current page is ended,switch no next page;if there's no more page,return false
    Re fetchNextRecord();

    ///@brief fetch record in current page
    Re fetchNextRecordInPage();
};
