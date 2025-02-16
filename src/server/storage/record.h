#pragma once
#include "../common/re.h"// for Re
#include "bitmap.h"      // for BitMap
#include "buffer_pool.h" // for BP_INVALID_PAGE_NUM, BufferPoolIterator
#include <cstdint>       // for int32_t
#include <string>        // for string
#include <unordered_set> // for unordered_set
int Align8(int size);
int PageHeaderSize();
int PageRecordCapacity(int page_data_size, int record_size);
int PageBitmapSize(int record_capacity);
int PageHeaderSize(int record_capacity);
struct RecordId {
public:
    int32_t page_id, slot_id;

public:
    RecordId() = default;
    RecordId(const int32_t &pid, const int32_t &sid) : page_id(pid), slot_id(sid) {}
    [[nodiscard]] std::string ToString() const;
    bool operator==(const RecordId &other) const { return page_id == other.page_id && slot_id == other.slot_id; }
    bool operator!=(const RecordId &other) const { return !(*this == other); }

public:
    static int Compare(const RecordId *rid_1, const RecordId *rid_2);
    static RecordId *Min();
    static RecordId *Max();
};
class Record
{
public:
    Record() : data_(nullptr) {}
    ///@brief set record_id and data of record
    void Init(const RecordId &record_id, char *data);
    void SetRecordId(const RecordId &record_id) { record_id_ = record_id; }
    void SetRecordId(const int32_t page_id, const int32_t slot_id) {
        this->record_id_.page_id = page_id;
        this->record_id_.slot_id = slot_id;
    }
    void SetData(char *data) { data_ = data; };
    [[nodiscard]] RecordId &GetRecordId() { return record_id_; }
    [[nodiscard]] char *GetData() { return data_; }

private:
    RecordId record_id_;
    char *data_;
};
class ConditionFilter;
///@brief there are some meta params in page header
///@NOTE page header is part of page's data
///@n actually,page header is composed of 2 parts--struct PageHeader and bitmap of the page
struct PageHeader {
    int32_t record_num;         // 当前页面记录的个数
    int32_t record_capacity;    // 最大记录个数
    int32_t record_real_size;   // 每条记录的实际大小
    int32_t record_size;        // 每条记录占用实际空间大小(可能对齐)
    int32_t first_record_offset;// 第一条记录的偏移量
};
class RecordPageHandler;
///@brief iterator of records in a page
class RecordPageIterator
{
public:
    RecordPageIterator() : record_page_handler_(nullptr), page_id_(BP_INVALID_PAGE_ID), next_slot_id_(0) {}
    void Init(RecordPageHandler &record_page_handler);
    bool HasNext();
    Re Next(class Record &record);
    [[nodiscard]] bool IsValid() const { return record_page_handler_ != nullptr; }

private:
    RecordPageHandler *record_page_handler_;
    int32_t page_id_;     // current page's page id
    int32_t next_slot_id_;// next slot id to return by func:next
    BitMap bitmap_;
};
class RecordPageHandler
{
public:
    RecordPageHandler() : disk_buffer_pool_(nullptr), frame_(nullptr), page_header_(nullptr), bitmap_(nullptr) {}
    ///@brief cleanUp() to destruct
    Re Destroy();
    ///@brief init
    Re Init(DiskBufferPool &buffer_pool, int32_t page_id);
    ///@brief recover init
    Re RecoverInit(DiskBufferPool &buffer_pool, int32_t page_id);
    ///@brief set target page to empty page(includes page header and page data) in buffer and flush it;
    Re InitEmptyPage(DiskBufferPool &buffer_pool, int32_t page_id, int record_size);
    ///@brief get page id of page in frame
    [[nodiscard]] int32_t GetPageId() const;
    ///@brief return whether or not current page is full
    [[nodiscard]] bool IsFull() const;
    ///@brief get target record's data
    char *GetRecordData(int32_t slot_id);
    ///@brief insert record to next usable slot
    Re InsertRecord(const char *data, RecordId *rid);
    ///@brief change param:rec's corresponding slot id with its data
    ///@NOTE use rec as a container of slot id and data
    Re UpdateRecord(class Record *rec);
    ///@brief delete record from the page(set corresponding bit of bitmap to 0)
    ///@NOTE delete record will not set space of record to 0 or so on,but just set bit to 0;
    ///@n when there is no record in the page,it will be disposed
    Re DeleteRecord(const RecordId *rid);
    ///@brief get record by record id(param:rid) and pass it by param:rec
    Re GetRecord(const RecordId *rid, class Record *rec);
    ///@brief recover insert record
    Re RecoverInsertRecord(const char *data, RecordId *rid);
    template<class RecordUpdater>
    Re UpdateRecordInPlace(const RecordId *rid, RecordUpdater updater);

private:
    DiskBufferPool *disk_buffer_pool_;
    Frame *frame_;
    PageHeader *page_header_;
    char *bitmap_;

private:
    friend class RecordPageIterator;
};
///@brief use record page handler
class RecordFileHandler
{
public:
    RecordFileHandler() = default;
    ///@brief link buffer of file with record file handler and add all free pages to free_pages_(initFreePages)
    Re Init(DiskBufferPool *buffer_pool);
    ///@brief set buffer_pool_ to nullptr,unlink the record file handler with buffer of file
    void Destroy();
    ///@brief init a record page handler and update record
    ///@NOTE rec->rid is record id of record we're going to update,rec->data is new data of record
    Re UpdateRecord(class Record *rec);
    ///@brief delete record with given record id
    Re DeleteRecord(const RecordId *rid);
    ///@brief insert record to free page
    Re InsertRecord(const char *data, int record_size, RecordId *rid);
    ///@brief recover insert record
    Re RecoverInsertRecord(const char *data, int record_size, RecordId *rid);
    ///@brief get record with given record id and pass it by param:rec
    Re GetRecord(const RecordId *rid, class Record *rec);
    template<class RecordUpdater>
    Re UpdateRecordInPlace(const RecordId *rid, RecordUpdater updater);

private:
    DiskBufferPool *disk_buffer_pool_ = nullptr;
    std::unordered_set<int32_t> free_pages_;// 没有填充满的页面集合
private:
    ///@brief add all pages that is not full to free_pages_
    Re InitFreePages();
};
class RecordFileScanner
{
public:
    RecordFileScanner() : disk_buffer_pool_(nullptr), condition_filter_(nullptr) {}
    ///@brief open a file and scan it,return all records fit the condition filter
    Re Init(DiskBufferPool &buffer_pool, ConditionFilter *condition_filter);
    ///@brief unlink buffer of file and condition filter with iterator
    Re Destroy();
    bool HasNext();
    Re Next(class Record &record);

private:
    DiskBufferPool *disk_buffer_pool_;
    BufferPoolIterator buffer_pool_iterator_;
    ConditionFilter *condition_filter_;
    RecordPageHandler record_page_handler_;
    RecordPageIterator record_page_iterator_;
    class Record next_record_;

private:
    ///@brief fetch record
    Re FetchNextRecord();
    ///@brief fetch record in current page
    Re FetchNextRecordInPage();
};
