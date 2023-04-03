#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include <cstdint>
#include <sstream>
#include "bitmap.h"
#include "buffer_pool.h"
#include "../common/re.h"


class RecordId {
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

    void initialize(const RecordId &record_id, char *data);

    void setRecordId(const RecordId &record_id) { record_id_ = record_id; }

    void setRecordId(const int32_t page_id, const int32_t slot_id) {
        this->record_id_.page_id = page_id;
        this->record_id_.slot_id = slot_id;
    }

    void setData(char *data) { data_ = data; };

    [[nodiscard]] RecordId &getRecordId() { return record_id_; }

    [[nodiscard]] const char *getData() const { return data_; }

private:
    RecordId record_id_;
    char *data_;
};

class ConditionFilter;

struct PageHeader {
    int32_t record_num;           // 当前页面记录的个数
    int32_t record_capacity;      // 最大记录个数
    int32_t record_real_size;     // 每条记录的实际大小
    int32_t record_size;          // 每条记录占用实际空间大小(可能对齐)
    int32_t first_record_offset;  // 第一条记录的偏移量
};

class RecordPageHandler;

class RecordPageIterator {
public:
    RecordPageIterator() : record_page_handler_(nullptr), page_num_(BP_INVALID_PAGE_NUM), next_slot_num_(0) {}

    void init(RecordPageHandler &record_page_handler);

    bool hasNext();

    Re next(class Record &record);

    [[nodiscard]] bool isValid() const { return record_page_handler_ != nullptr; }

private:
    RecordPageHandler *record_page_handler_;
    int32_t page_num_;
    BitMap bitmap_;
    int32_t next_slot_num_;
};

class RecordPageHandler {
public:
    RecordPageHandler() : disk_buffer_pool_(nullptr), frame_(nullptr), page_header_(nullptr), bitmap_(nullptr) {}

    ~RecordPageHandler();

    Re init(DiskBufferPool &buffer_pool, int32_t page_id);

    Re recoverInit(DiskBufferPool &buffer_pool, int32_t page_id);

    Re initEmptyPage(DiskBufferPool &buffer_pool, int32_t page_id, int record_size);

    Re cleanUp();

    Re insertRecord(const char *data, RecordId *rid);

    Re recoverInsertRecord(const char *data, RecordId *rid);

    Re updateRecord(class Record *rec);

    template<class RecordUpdater>
    Re updateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
        class Record record;
        Re r = getRecord(rid, &record);
        if (r != Re::Success) {
            return r;
        }
        r = updater(record);
        frame_->dirtyMark();
        return r;
    }

    Re deleteRecord(const RecordId *rid);

    Re getRecord(const RecordId *rid, class Record *rec);

    [[nodiscard]] int32_t getPageNum() const;

    [[nodiscard]] bool isFull() const;

    char *getRecordData(int32_t slot_num) {
        return frame_->getPageData() + page_header_->first_record_offset + (page_header_->record_size * slot_num);
    }

private:
    DiskBufferPool *disk_buffer_pool_;
    Frame *frame_;
    PageHeader *page_header_;
    char *bitmap_;

private:
    friend class RecordPageIterator;
};

class RecordFileHandler {
public:
    RecordFileHandler() = default;

    Re init(DiskBufferPool *buffer_pool);

    void close();

    /**
     * 更新指定文件中的记录，rec指向的记录结构中的rid字段为要更新的记录的标识符，
     * pData字段指向新的记录内容
     */
    Re updateRecord(class Record *rec);

    /**
     * 从指定文件中删除标识符为rid的记录
     */
    Re deleteRecord(const RecordId *rid);

    /**
     * 插入一个新的记录到指定文件中，pData为指向新纪录内容的指针，返回该记录的标识符rid
     */
    Re insertRecord(const char *data, int record_size, RecordId *rid);

    Re recoverInsertRecord(const char *data, int record_size, RecordId *rid);

    /**
     * 获取指定文件中标识符为rid的记录内容到rec指向的记录结构中
     */
    Re getRecord(const RecordId *rid, class Record *rec);

    template<class RecordUpdater>
    // 改成普通模式, 不使用模板
    Re updateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
        RecordPageHandler page_handler;
        Re r = page_handler.init(*disk_buffer_pool_, rid->page_id);
        if (r != Re::Success)
            return r;
        return page_handler.updateRecordInPlace(rid, updater);
    }


private:
    DiskBufferPool *disk_buffer_pool_ = nullptr;
    std::unordered_set<int32_t> free_pages_; // 没有填充满的页面集合

private:
    Re initFreePages();
};

class RecordFileScanner {
public:
    RecordFileScanner() : disk_buffer_pool_(nullptr), condition_filter_(nullptr) {}

    /**
     * 打开一个文件扫描。
     * 如果条件不为空，则要对每条记录进行条件比较，只有满足所有条件的记录才被返回
     */
    Re openScan(DiskBufferPool &buffer_pool, ConditionFilter *condition_filter);

    /**
     * 关闭一个文件扫描，释放相应的资源
     */
    Re closeScan();

    bool hasNext();

    Re next(class Record &record);

private:
    DiskBufferPool *disk_buffer_pool_;
    BufferPoolIterator bp_iterator_;
    ConditionFilter *condition_filter_;
    RecordPageHandler record_page_handler_;
    RecordPageIterator record_page_iterator_;
    class Record next_record_;
private:
    Re fetchNextRecord();

    Re fetchNextRecordInPage();
};
