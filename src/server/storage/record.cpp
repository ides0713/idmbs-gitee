#include "record.h"
#include "condition_filter.h"

///@brief calculate how many bit data(size=param:size) will use (unit is byte(8 bit))
int align8(int size) {
    return size / 8 * 8 + ((size % 8 == 0) ? 0 : 8);
}

///@brief return size of struct PageHeader of a page
int pageHeaderSize() {
    return sizeof(PageHeader);
}

///@brief page record capacity
int pageRecordCapacity(int page_data_size, int record_size) {
    return (int) ((page_data_size - pageHeaderSize() - 1) / (record_size + 0.125));
}

///@brief
int pageBitmapSize(int record_capacity) {
    return record_capacity / 8 + ((record_capacity % 8 == 0) ? 0 : 1);
}

///@brief header of a page is composed of two parts
int pageHeaderSize(int record_capacity) {
    const int bitmap_size = pageBitmapSize(record_capacity);
    return align8(pageHeaderSize() + bitmap_size);
}

std::string RecordId::toString() const {
    std::stringstream ss;
    ss << "PageId:" << page_id << ", SlotId:" << slot_id;
    return ss.str();
}

int RecordId::compare(const RecordId *rid_1, const RecordId *rid_2) {
    return rid_1->page_id - rid_2->page_id == 0 ? rid_1->page_id - rid_2->page_id : rid_1->slot_id - rid_2->slot_id;
}

RecordId *RecordId::min() {
    static RecordId rid{0, 0};
    return &rid;
}

RecordId *RecordId::max() {
    static RecordId rid{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()};
    return &rid;
}

void Record::init(const RecordId &record_id, char *data) {
    setRecordId(record_id);
    setData(data);
}

void RecordPageIterator::init(RecordPageHandler &record_page_handler) {
    record_page_handler_ = &record_page_handler;
    page_id_ = record_page_handler.getPageId();
    bitmap_.init(record_page_handler.bitmap_, record_page_handler.page_header_->record_capacity);
    next_slot_id_ = bitmap_.nextSetBit(0);
}

bool RecordPageIterator::hasNext() {
    return next_slot_id_ != -1;
}

Re RecordPageIterator::next(class Record &record) {
    record.setRecordId(page_id_, next_slot_id_);
    record.setData(record_page_handler_->getRecordData(record.getRecordId().slot_id));
    if (next_slot_id_ >= 0)
        next_slot_id_ = bitmap_.nextSetBit(next_slot_id_ + 1);
    return record.getRecordId().slot_id != -1 ? Re::Success : Re::RecordEof;
}

Re RecordPageHandler::destroy() {
    if (disk_buffer_pool_ != nullptr) {
        disk_buffer_pool_->unpinPage(frame_);
        disk_buffer_pool_ = nullptr;
    }
    return Re::Success;
}

Re RecordPageHandler::init(DiskBufferPool &buffer_pool, int32_t page_id) {
    if (disk_buffer_pool_ != nullptr) {
        debugPrint("RecordPageHandler:disk buffer pool has been opened for page_id %d.\n", page_id);
        return Re::RecordOpened;
    }
    Re r = buffer_pool.getPage(page_id, &frame_);
    if (r != Re::Success) {
        debugPrint("RecordPageHandler:failed to getFrame page handle from disk buffer pool. ret=%d\n", r);
        return r;
    }
    char *data = frame_->getPageData();
    disk_buffer_pool_ = &buffer_pool;
    page_header_ = (PageHeader *) (data);
    bitmap_ = data + pageHeaderSize();
    debugPrint("RecordPageHandler:successfully init page_id %d.\n", page_id);
    return Re::Success;
}

Re RecordPageHandler::recoverInit(DiskBufferPool &buffer_pool, int32_t page_id) {
    if (disk_buffer_pool_ != nullptr) {
        debugPrint("RecordPageHandler:disk buffer pool has been opened for page_id %d.\n", page_id);
        return Re::RecordOpened;
    }
    Re r = buffer_pool.getPage(page_id, &frame_);
    if (r != Re::Success) {
        debugPrint("RecordPageHandler:failed to getFrame page handle from disk buffer pool. re=%d\n", r);
        return r;
    }
    char *data = frame_->getPageData();
    disk_buffer_pool_ = &buffer_pool;
    page_header_ = (PageHeader *) (data);
    bitmap_ = data + pageHeaderSize();
    buffer_pool.recoverPage(page_id);
    debugPrint("RecordPageHandler:successfully recover init page_id %d.\n", page_id);
    return Re::Success;
}

Re RecordPageHandler::initEmptyPage(DiskBufferPool &buffer_pool, int32_t page_id, int record_size) {
    Re r = init(buffer_pool, page_id);
    if (r != Re::Success) {
        debugPrint("RecordPageHandler:failed to init empty page page_id:record_size %d:%d.\n", page_id, record_size);
        return r;
    }
    int page_size = BP_PAGE_DATA_SIZE, record_phy_size = align8(record_size);
    page_header_->record_num = 0;
    page_header_->record_capacity = pageRecordCapacity(page_size, record_phy_size);
    page_header_->record_real_size = record_size;
    page_header_->record_size = record_phy_size;
    page_header_->first_record_offset = pageHeaderSize(page_header_->record_capacity);
    bitmap_ = frame_->getPageData() + pageHeaderSize();
    memset(bitmap_, 0, pageBitmapSize(page_header_->record_capacity));
    r = buffer_pool.flushPage(*frame_);
    if (r != Re::Success) {
        debugPrint("RecordPageHandler:failed to flush page header %d.\n", page_id);
        return r;
    }
    return Re::Success;
}

Re RecordPageHandler::insertRecord(const char *data, RecordId *rid) {
    if (page_header_->record_num == page_header_->record_capacity) {
        debugPrint("RecordPageHandler:page is full, page_id %d.\n", frame_->getPageId());
        return Re::RecordNoMem;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    int slot_id = bitmap.nextUnsetBit(0);
    bitmap.setBit(slot_id);
    page_header_->record_num++;
    char *record_data = getRecordData(slot_id);
    memcpy(record_data, data, page_header_->record_real_size);
    frame_->dirtyMark();
    if (rid) {
        rid->page_id = getPageId();
        rid->slot_id = slot_id;
    }
    return Re::Success;
}

Re RecordPageHandler::recoverInsertRecord(const char *data, RecordId *rid) {
    if (page_header_->record_num == page_header_->record_capacity) {
        debugPrint("RecordPageHandler:page is full, page_id %d.\n", frame_->getPageId());
        return Re::RecordNoMem;
    }
    if (rid->slot_id >= page_header_->record_capacity) {
        debugPrint("RecordPageHandler:slot_id illegal, slot_num(%d) > record_capacity(%d).\n", rid->slot_id,
                   page_header_->record_capacity);
        return Re::RecordNoMem;
    }
    // 更新位图
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    bitmap.setBit(rid->slot_id);
    page_header_->record_num++;
    // 恢复数据
    char *record_data = getRecordData(rid->slot_id);
    memcpy(record_data, data, page_header_->record_real_size);
    frame_->dirtyMark();
    return Re::Success;
}

Re RecordPageHandler::updateRecord(class Record *rec) {
    if (rec->getRecordId().slot_id >= page_header_->record_capacity) {
        debugPrint("RecordPageHandler:invalid slot_id %d, exceed page's record capacity, page_id %d.\n",
                   rec->getRecordId().slot_id, frame_->getPageId());
        return Re::InvalidArgument;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.getBit(rec->getRecordId().slot_id)) {
        debugPrint("RecordPageHandler:invalid slot_id %d, slot is empty, page_id %d.\n",
                   rec->getRecordId().slot_id, frame_->getPageId());
        return Re::RecordRecordNotExist;
    } else {
        char *record_data = getRecordData(rec->getRecordId().slot_id);
        memcpy(record_data, rec->getData(), page_header_->record_real_size);
        bitmap.setBit(rec->getRecordId().slot_id);
        frame_->dirtyMark();
        return Re::Success;
    }
}

Re RecordPageHandler::deleteRecord(const RecordId *rid) {
    if (rid->slot_id >= page_header_->record_capacity) {
        debugPrint("RecordPageHandler:invalid slot_id %d, exceed page's record capacity, page_id %d.\n",
                   rid->slot_id, frame_->getPageId());
        return Re::InvalidArgument;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (bitmap.getBit(rid->slot_id)) {
        bitmap.clearBit(rid->slot_id);
        page_header_->record_num--;
        frame_->dirtyMark();
        if (page_header_->record_num == 0) {
            //if there is no record in the page,just dispose it
            DiskBufferPool *disk_buffer_pool = disk_buffer_pool_;
            int32_t page_num = getPageId();
            destroy();
            disk_buffer_pool->disposePage(page_num);
        }
        return Re::Success;
    } else {
        debugPrint("RecordPageHandler:invalid slot_id %d, slot is empty, page_id %d.\n",
                   rid->slot_id, frame_->getPageId());
        return Re::RecordRecordNotExist;
    }
}

Re RecordPageHandler::getRecord(const RecordId *rid, class Record *rec) {
    if (rid->slot_id >= page_header_->record_capacity) {
        debugPrint("RecordPageHandler:invalid slot_id:%d, exceed page's record capacity, page_id %d.\n",
                   rid->slot_id, frame_->getPageId());
        return Re::RecordInvalidRId;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.getBit(rid->slot_id)) {
        debugPrint("RecordPageHandler:invalid slot_id:%d, slot is empty, page_id %d.\n",
                   rid->slot_id, frame_->getPageId());
        return Re::RecordRecordNotExist;
    }
    rec->setRecordId(*rid);
    rec->setData(getRecordData(rid->slot_id));
    return Re::Success;
}

int32_t RecordPageHandler::getPageId() const {
    if (page_header_ == nullptr)
        return (int32_t) (-1);
    return frame_->getPageId();
}

bool RecordPageHandler::isFull() const {
    return page_header_->record_num >= page_header_->record_capacity;
}

char *RecordPageHandler::getRecordData(int32_t slot_id) {
    return frame_->getPageData() + page_header_->first_record_offset + (page_header_->record_size * slot_id);
}


template<class RecordUpdater>
Re RecordPageHandler::updateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
    class Record record;
    Re r = getRecord(rid, &record);
    if (r != Re::Success)
        return r;
    r = updater(record);
    frame_->dirtyMark();
    return r;
}


Re RecordFileHandler::init(DiskBufferPool *buffer_pool) {
    if (disk_buffer_pool_ != nullptr) {
        debugPrint("RecordFileHandler:record file handler has been opened.\n");
        return Re::RecordOpened;
    }
    disk_buffer_pool_ = buffer_pool;
    Re r = initFreePages();
    debugPrint("RecordFileHandler:open record file handle done. r=%d\n", r);
    return Re::Success;
}

void RecordFileHandler::destroy() {
    if (disk_buffer_pool_ != nullptr)
        disk_buffer_pool_ = nullptr;
}

Re RecordFileHandler::initFreePages() {
    // 遍历当前文件上所有页面，找到没有满的页面
    BufferPoolIterator bp_iterator;
    bp_iterator.init(*disk_buffer_pool_);
    RecordPageHandler record_page_handler;
    int32_t current_page_id = 0;
    while (bp_iterator.hasNext()) {
        current_page_id = bp_iterator.next();
        Re r = record_page_handler.init(*disk_buffer_pool_, current_page_id);
        if (r != Re::Success) {
            debugPrint("RecordFileHandler:failed to init record page handler. page id=%d, r=%d.\n",
                       current_page_id, r);
            return r;
        }
        if (!record_page_handler.isFull())
            free_pages_.insert(current_page_id);
        record_page_handler.destroy();
    }
    return Re::Success;
}

Re RecordFileHandler::updateRecord(class Record *rec) {
    RecordPageHandler page_handler;
    Re r = page_handler.init(*disk_buffer_pool_, rec->getRecordId().page_id);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to init record page handler.page id=%d\n", rec->getRecordId().page_id);
        return r;
    }
    return page_handler.updateRecord(rec);
}

Re RecordFileHandler::deleteRecord(const RecordId *rid) {
    RecordPageHandler page_handler;
    Re r = page_handler.init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to init record page handler.page id=%d. r=%d\n", rid->page_id, r);
        return r;
    }
    r = page_handler.deleteRecord(rid);
    if (r == Re::Success)
        free_pages_.insert(rid->page_id);
    return r;
}

Re RecordFileHandler::insertRecord(const char *data, int record_size, RecordId *rid) {
    // try to find page not full in free_pages_
    RecordPageHandler record_page_handler;
    int32_t current_page_id = 0;
    while (!free_pages_.empty()) {
        current_page_id = *free_pages_.begin();
        Re r = record_page_handler.init(*disk_buffer_pool_, current_page_id);
        if (r != Re::Success) {
            debugPrint("RecordFileHandler:failed to init record page handler. page id=%d, r=%d\n", current_page_id,
                       r);
            return r;
        }
        if (!record_page_handler.isFull())
            return record_page_handler.insertRecord(data, rid);
        record_page_handler.destroy();
        free_pages_.erase(free_pages_.begin());
    }
    //allocate a new page if can not find page not full in free_pages_
    Frame *frame = nullptr;
    Re r = disk_buffer_pool_->allocatePage(&frame);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to allocate page while inserting record. r:%d.\n", r);
        return r;
    }
    current_page_id = frame->getPageId();
    r = record_page_handler.initEmptyPage(*disk_buffer_pool_, current_page_id, record_size);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to init empty page. r:%d.\n", r);
        if (disk_buffer_pool_->unpinPage(frame) != Re::Success)
            debugPrint("RecordFileHandler:failed to unpin page.\n");
        return r;
    }
    disk_buffer_pool_->unpinPage(frame);
    free_pages_.insert(current_page_id);
    return record_page_handler.insertRecord(data, rid);
}

Re RecordFileHandler::recoverInsertRecord(const char *data, int record_size, RecordId *rid) {
    RecordPageHandler record_page_handler;
    Re r = record_page_handler.recoverInit(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to init record page handler. page id=%d, rc=%d.\n", rid->page_id, r);
        return r;
    }
    return record_page_handler.recoverInsertRecord(data, rid);
}

Re RecordFileHandler::getRecord(const RecordId *rid, class Record *rec) {
    // lock?
    if (rid == nullptr or rec == nullptr) {
        debugPrint("RecordFiledHandler:invalid rid %p or rec %p, one of them is null.\n", rid, rec);
        return Re::InvalidArgument;
    }
    RecordPageHandler page_handler;
    Re r = page_handler.init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        debugPrint("RecordFileHandler:failed to init record page handler.page number=%d\n", rid->page_id);
        return r;
    }
    return page_handler.getRecord(rid, rec);
}

template<class RecordUpdater>
Re RecordFileHandler::updateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
    RecordPageHandler page_handler;
    Re r = page_handler.init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success)
        return r;
    return page_handler.updateRecordInPlace(rid, updater);
}

Re RecordFileScanner::init(DiskBufferPool &buffer_pool, ConditionFilter *condition_filter) {
    destroy();
    disk_buffer_pool_ = &buffer_pool;
    Re r = buffer_pool_iterator_.init(buffer_pool);
    if (r != Re::Success) {
        debugPrint("RecordFileScanner:failed to init bp iterator. r=%d.\n", r);
        return r;
    }
    condition_filter_ = condition_filter;
    r = fetchNextRecord();
    if (r == Re::RecordEof)
        return Re::Success;
    return r;
}

Re RecordFileScanner::destroy() {
    if (disk_buffer_pool_ != nullptr)
        disk_buffer_pool_ = nullptr;
    if (condition_filter_ != nullptr)
        condition_filter_ = nullptr;
    return Re::Success;
}

bool RecordFileScanner::hasNext() {
    return false;
}

Re RecordFileScanner::next(class Record &record) {
    record = next_record_;
    Re rc = fetchNextRecord();
    if (rc == Re::RecordEof)
        rc = Re::Success;
    return rc;
}

Re RecordFileScanner::fetchNextRecord() {
    if (record_page_iterator_.isValid()) {
        Re r = fetchNextRecordInPage();
        if (r != Re::RecordEof)
            return r;
    }
    //this page is 'end',re-init record page handler
    while (buffer_pool_iterator_.hasNext()) {
        int32_t page_id = buffer_pool_iterator_.next();
        record_page_handler_.destroy();
        Re r = record_page_handler_.init(*disk_buffer_pool_, page_id);
        if (r != Re::Success) {
            debugPrint("RecordFileScanner:failed to init record page handler. r=%d:%s\n", r, strRe(r));
            return r;
        }
        record_page_iterator_.init(record_page_handler_);
        r = fetchNextRecordInPage();
        if (r != Re::RecordEof)
            return r;
    }
    next_record_.getRecordId().slot_id = -1;
    return Re::RecordEof;
}

Re RecordFileScanner::fetchNextRecordInPage() {
    while (record_page_iterator_.hasNext()) {
        Re r = record_page_iterator_.next(next_record_);
        if (r != Re::Success)
            return r;
        if (condition_filter_ == nullptr or condition_filter_->filter(next_record_))
            return Re::Success;
        next_record_.getRecordId().slot_id = -1;
        return Re::RecordEof;
    }
}
