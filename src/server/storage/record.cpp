#include "record.h"
#include "condition_filter.h"
///@brief calculate how many bit data(size=param:size) will use (unit is byte(8 bit))
int Align8(int size) {
    return size / 8 * 8 + ((size % 8 == 0) ? 0 : 8);
}
///@brief return size of struct PageHeader of a page
int PageHeaderSize() {
    return sizeof(PageHeader);
}
///@brief page record capacity
int PageRecordCapacity(int page_data_size, int record_size) {
    return int((page_data_size - PageHeaderSize() - 1) / (record_size + 0.125));
}
///@brief
int PageBitmapSize(int record_capacity) {
    return record_capacity / 8 + ((record_capacity % 8 == 0) ? 0 : 1);
}
///@brief header of a page is composed of two parts
int PageHeaderSize(int record_capacity) {
    const int bitmap_size = PageBitmapSize(record_capacity);
    return Align8(PageHeaderSize() + bitmap_size);
}
std::string RecordId::ToString() const {
    std::stringstream ss;
    ss << "PageId:" << page_id << ", SlotId:" << slot_id;
    return ss.str();
}
int RecordId::Compare(const RecordId *rid_1, const RecordId *rid_2) {
    return rid_1->page_id - rid_2->page_id == 0 ? rid_1->page_id - rid_2->page_id : rid_1->slot_id - rid_2->slot_id;
}
RecordId *RecordId::Min() {
    static RecordId rid{0, 0};
    return &rid;
}
RecordId *RecordId::Max() {
    static RecordId rid{std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()};
    return &rid;
}
void Record::Init(const RecordId &record_id, char *data) {
    SetRecordId(record_id);
    SetData(data);
}
void RecordPageIterator::Init(RecordPageHandler &record_page_handler) {
    record_page_handler_ = &record_page_handler;
    page_id_ = record_page_handler.GetPageId();
    bitmap_.Init(record_page_handler.bitmap_, record_page_handler.page_header_->record_capacity);
    next_slot_id_ = bitmap_.NextSetBit(0);
}
bool RecordPageIterator::HasNext() {
    return next_slot_id_ != -1;
}
Re RecordPageIterator::Next(class Record &record) {
    record.SetRecordId(page_id_, next_slot_id_);
    record.SetData(record_page_handler_->GetRecordData(record.GetRecordId().slot_id));
    if (next_slot_id_ >= 0)
        next_slot_id_ = bitmap_.NextSetBit(next_slot_id_ + 1);
    return record.GetRecordId().slot_id != -1 ? Re::Success : Re::RecordEof;
}
Re RecordPageHandler::Destroy() {
    if (disk_buffer_pool_ != nullptr) {
        disk_buffer_pool_->UnpinPage(frame_);
        disk_buffer_pool_ = nullptr;
    }
    return Re::Success;
}
Re RecordPageHandler::Init(DiskBufferPool &buffer_pool, int32_t page_id) {
    if (disk_buffer_pool_ != nullptr) {
        DebugPrint("RecordPageHandler:disk buffer pool has been opened for page_id %d.\n", page_id);
        return Re::RecordOpened;
    }
    Re r = buffer_pool.GetPage(page_id, &frame_);
    if (r != Re::Success) {
        DebugPrint("RecordPageHandler:failed to getFrame page handle from disk buffer pool. ret=%d\n", r);
        return r;
    }
    char *data = frame_->GetPageData();
    disk_buffer_pool_ = &buffer_pool;
    page_header_ = reinterpret_cast<PageHeader *>(data);
    bitmap_ = data + PageHeaderSize();
    DebugPrint("RecordPageHandler:successfully init page_id %d.\n", page_id);
    return Re::Success;
}
Re RecordPageHandler::RecoverInit(DiskBufferPool &buffer_pool, int32_t page_id) {
    if (disk_buffer_pool_ != nullptr) {
        DebugPrint("RecordPageHandler:disk buffer pool has been opened for page_id %d.\n", page_id);
        return Re::RecordOpened;
    }
    Re r = buffer_pool.GetPage(page_id, &frame_);
    if (r != Re::Success) {
        DebugPrint("RecordPageHandler:failed to getFrame page handle from disk buffer pool. re=%d\n", r);
        return r;
    }
    char *data = frame_->GetPageData();
    disk_buffer_pool_ = &buffer_pool;
    page_header_ = (PageHeader *) (data);
    bitmap_ = data + PageHeaderSize();
    buffer_pool.RecoverPage(page_id);
    DebugPrint("RecordPageHandler:successfully recover init page_id %d.\n", page_id);
    return Re::Success;
}
Re RecordPageHandler::InitEmptyPage(DiskBufferPool &buffer_pool, int32_t page_id, int record_size) {
    Re r = Init(buffer_pool, page_id);
    if (r != Re::Success) {
        DebugPrint("RecordPageHandler:failed to init empty page page_id:record_size %d:%d.\n", page_id, record_size);
        return r;
    }
    int page_size = BP_PAGE_DATA_SIZE, record_phy_size = Align8(record_size);
    page_header_->record_num = 0;
    page_header_->record_capacity = PageRecordCapacity(page_size, record_phy_size);
    page_header_->record_real_size = record_size;
    page_header_->record_size = record_phy_size;
    page_header_->first_record_offset = PageHeaderSize(page_header_->record_capacity);
    bitmap_ = frame_->GetPageData() + PageHeaderSize();
    memset(bitmap_, 0, PageBitmapSize(page_header_->record_capacity));
    r = buffer_pool.FlushPage(*frame_);
    if (r != Re::Success) {
        DebugPrint("RecordPageHandler:failed to flush page header %d.\n", page_id);
        return r;
    }
    return Re::Success;
}
Re RecordPageHandler::InsertRecord(const char *data, RecordId *rid) {
    if (page_header_->record_num == page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:page is full, page_id %d.\n", frame_->GetPageId());
        return Re::RecordNoMem;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    int slot_id = bitmap.NextUnsetBit(0);
    bitmap.SetBit(slot_id);
    page_header_->record_num++;
    char *record_data = GetRecordData(slot_id);
    memmove(record_data, data, page_header_->record_real_size);
    frame_->DirtyMark();
    if (rid != nullptr) {
        rid->page_id = GetPageId();
        rid->slot_id = slot_id;
    }
    return Re::Success;
}
Re RecordPageHandler::RecoverInsertRecord(const char *data, RecordId *rid) {
    if (page_header_->record_num == page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:page is full, page_id %d.\n", frame_->GetPageId());
        return Re::RecordNoMem;
    }
    if (rid->slot_id >= page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:slot_id illegal, slot_num(%d) > record_capacity(%d).\n", rid->slot_id,
                   page_header_->record_capacity);
        return Re::RecordNoMem;
    }
    // 更新位图
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    bitmap.SetBit(rid->slot_id);
    page_header_->record_num++;
    // 恢复数据
    char *record_data = GetRecordData(rid->slot_id);
    memcpy(record_data, data, page_header_->record_real_size);
    frame_->DirtyMark();
    return Re::Success;
}
Re RecordPageHandler::UpdateRecord(class Record *rec) {
    if (rec->GetRecordId().slot_id >= page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:invalid slot_id %d, exceed page's record capacity, page_id %d.\n",
                   rec->GetRecordId().slot_id, frame_->GetPageId());
        return Re::InvalidArgument;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.GetBit(rec->GetRecordId().slot_id)) {
        DebugPrint("RecordPageHandler:invalid slot_id %d, slot is empty, page_id %d.\n", rec->GetRecordId().slot_id,
                   frame_->GetPageId());
        return Re::RecordRecordNotExist;
    } else {
        char *record_data = GetRecordData(rec->GetRecordId().slot_id);
        memcpy(record_data, rec->GetData(), page_header_->record_real_size);
        bitmap.SetBit(rec->GetRecordId().slot_id);
        frame_->DirtyMark();
        return Re::Success;
    }
}
Re RecordPageHandler::DeleteRecord(const RecordId *rid) {
    if (rid->slot_id >= page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:invalid slot_id %d, exceed page's record capacity, page_id %d.\n", rid->slot_id,
                   frame_->GetPageId());
        return Re::InvalidArgument;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (bitmap.GetBit(rid->slot_id)) {
        bitmap.ClearBit(rid->slot_id);
        page_header_->record_num--;
        frame_->DirtyMark();
        if (page_header_->record_num == 0) {
            // if there is no record in the page,just dispose it
            DiskBufferPool *disk_buffer_pool = disk_buffer_pool_;
            int32_t page_num = GetPageId();
            Destroy();
            disk_buffer_pool->DisposePage(page_num);
        }
        return Re::Success;
    } else {
        DebugPrint("RecordPageHandler:invalid slot_id %d, slot is empty, page_id %d.\n", rid->slot_id,
                   frame_->GetPageId());
        return Re::RecordRecordNotExist;
    }
}
Re RecordPageHandler::GetRecord(const RecordId *rid, class Record *rec) {
    if (rid->slot_id >= page_header_->record_capacity) {
        DebugPrint("RecordPageHandler:invalid slot_id:%d, exceed page's record capacity, page_id %d.\n", rid->slot_id,
                   frame_->GetPageId());
        return Re::RecordInvalidRId;
    }
    BitMap bitmap(bitmap_, page_header_->record_capacity);
    if (!bitmap.GetBit(rid->slot_id)) {
        DebugPrint("RecordPageHandler:invalid slot_id:%d, slot is empty, page_id %d.\n", rid->slot_id,
                   frame_->GetPageId());
        return Re::RecordRecordNotExist;
    }
    rec->SetRecordId(*rid);
    rec->SetData(GetRecordData(rid->slot_id));
    return Re::Success;
}
int32_t RecordPageHandler::GetPageId() const {
    if (page_header_ == nullptr)
        return (int32_t) (-1);
    return frame_->GetPageId();
}
bool RecordPageHandler::IsFull() const {
    return page_header_->record_num >= page_header_->record_capacity;
}
char *RecordPageHandler::GetRecordData(int32_t slot_id) {
    return frame_->GetPageData() + page_header_->first_record_offset + (page_header_->record_size * slot_id);
}
template<class RecordUpdater>
Re RecordPageHandler::UpdateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
    class Record record;
    Re r = GetRecord(rid, &record);
    if (r != Re::Success)
        return r;
    r = updater(record);
    frame_->DirtyMark();
    return r;
}
Re RecordFileHandler::Init(DiskBufferPool *buffer_pool) {
    if (disk_buffer_pool_ != nullptr) {
        DebugPrint("RecordFileHandler:record file handler has been opened.\n");
        return Re::RecordOpened;
    }
    disk_buffer_pool_ = buffer_pool;
    Re r = InitFreePages();
    DebugPrint("RecordFileHandler:open record file handle done. r=%d\n", r);
    return Re::Success;
}
void RecordFileHandler::Destroy() {
    if (disk_buffer_pool_ != nullptr)
        disk_buffer_pool_ = nullptr;
}
Re RecordFileHandler::InitFreePages() {
    // 遍历当前文件上所有页面，找到没有满的页面
    BufferPoolIterator bp_iterator;
    bp_iterator.Init(*disk_buffer_pool_);
    RecordPageHandler record_page_handler;
    int32_t current_page_id = 0;
    while (bp_iterator.HasNext()) {
        current_page_id = bp_iterator.Next();
        Re r = record_page_handler.Init(*disk_buffer_pool_, current_page_id);
        if (r != Re::Success) {
            DebugPrint("RecordFileHandler:failed to init record page handler. page id=%d, r=%d.\n", current_page_id, r);
            return r;
        }
        if (!record_page_handler.IsFull())
            free_pages_.insert(current_page_id);
        record_page_handler.Destroy();
    }
    return Re::Success;
}
Re RecordFileHandler::UpdateRecord(class Record *rec) {
    RecordPageHandler page_handler;
    Re r = page_handler.Init(*disk_buffer_pool_, rec->GetRecordId().page_id);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to init record page handler.page id=%d\n", rec->GetRecordId().page_id);
        return r;
    }
    return page_handler.UpdateRecord(rec);
}
Re RecordFileHandler::DeleteRecord(const RecordId *rid) {
    RecordPageHandler page_handler;
    Re r = page_handler.Init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to init record page handler.page id=%d. r=%d\n", rid->page_id, r);
        return r;
    }
    r = page_handler.DeleteRecord(rid);
    if (r == Re::Success)
        free_pages_.insert(rid->page_id);
    return r;
}
Re RecordFileHandler::InsertRecord(const char *data, int record_size, RecordId *rid) {
    // try to find page not full in free_pages_
    RecordPageHandler record_page_handler;
    int32_t current_page_id = 0;
    while (!free_pages_.empty()) {
        current_page_id = *free_pages_.begin();
        Re r = record_page_handler.Init(*disk_buffer_pool_, current_page_id);
        if (r != Re::Success) {
            DebugPrint("RecordFileHandler:failed to init record page handler. page id=%d, r=%d\n", current_page_id, r);
            return r;
        }
        if (!record_page_handler.IsFull())
            return record_page_handler.InsertRecord(data, rid);
        record_page_handler.Destroy();
        free_pages_.erase(free_pages_.begin());
    }
    // allocate a new page if can not find page not full in free_pages_
    Frame *frame = nullptr;
    Re r = disk_buffer_pool_->AllocatePage(&frame);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to allocate page while inserting record. r:%d.\n", r);
        return r;
    }
    current_page_id = frame->GetPageId();
    r = record_page_handler.InitEmptyPage(*disk_buffer_pool_, current_page_id, record_size);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to init empty page. r:%d.\n", r);
        if (disk_buffer_pool_->UnpinPage(frame) != Re::Success)
            DebugPrint("RecordFileHandler:failed to unpin page.\n");
        return r;
    }
    disk_buffer_pool_->UnpinPage(frame);
    free_pages_.insert(current_page_id);
    printf("before page handler handle(insert record)\n");
    return record_page_handler.InsertRecord(data, rid);
}
Re RecordFileHandler::RecoverInsertRecord(const char *data, int record_size, RecordId *rid) {
    RecordPageHandler record_page_handler;
    Re r = record_page_handler.RecoverInit(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to init record page handler. page id=%d, rc=%d.\n", rid->page_id, r);
        return r;
    }
    return record_page_handler.RecoverInsertRecord(data, rid);
}
Re RecordFileHandler::GetRecord(const RecordId *rid, class Record *rec) {
    // lock?
    if (rid == nullptr or rec == nullptr) {
        DebugPrint("RecordFiledHandler:invalid rid %p or rec %p, one of them is null.\n", rid, rec);
        return Re::InvalidArgument;
    }
    RecordPageHandler page_handler;
    Re r = page_handler.Init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success) {
        DebugPrint("RecordFileHandler:failed to init record page handler.page number=%d\n", rid->page_id);
        return r;
    }
    return page_handler.GetRecord(rid, rec);
}
template<class RecordUpdater>
Re RecordFileHandler::UpdateRecordInPlace(const RecordId *rid, RecordUpdater updater) {
    RecordPageHandler page_handler;
    Re r = page_handler.Init(*disk_buffer_pool_, rid->page_id);
    if (r != Re::Success)
        return r;
    return page_handler.UpdateRecordInPlace(rid, updater);
}
Re RecordFileScanner::Init(DiskBufferPool &buffer_pool, ConditionFilter *condition_filter) {
    Destroy();
    disk_buffer_pool_ = &buffer_pool;
    Re r = buffer_pool_iterator_.Init(buffer_pool);
    if (r != Re::Success) {
        DebugPrint("RecordFileScanner:failed to init bp iterator. r=%d.\n", r);
        return r;
    }
    condition_filter_ = condition_filter;
    r = FetchNextRecord();
    if (r == Re::RecordEof)
        return Re::Success;
    return r;
}
Re RecordFileScanner::Destroy() {
    if (disk_buffer_pool_ != nullptr)
        disk_buffer_pool_ = nullptr;
    if (condition_filter_ != nullptr)
        condition_filter_ = nullptr;
    return Re::Success;
}
bool RecordFileScanner::HasNext() {
    return next_record_.GetRecordId().slot_id != -1;
}
Re RecordFileScanner::Next(class Record &record) {
    record = next_record_;
    Re r = FetchNextRecord();
    if (r == Re::RecordEof)
        r = Re::Success;
    return r;
}
Re RecordFileScanner::FetchNextRecord() {
    if (record_page_iterator_.IsValid()) {
        Re r = FetchNextRecordInPage();
        if (r != Re::RecordEof)
            return r;
    }
    // this page is 'end',re-init record page handler
    while (buffer_pool_iterator_.HasNext()) {
        int32_t page_id = buffer_pool_iterator_.Next();
        record_page_handler_.Destroy();
        // todo check here
        Re r = record_page_handler_.Init(*disk_buffer_pool_, page_id);
        if (r != Re::Success) {
            DebugPrint("RecordFileScanner:failed to init record page handler. r=%d:%s\n", r, StrRe(r));
            return r;
        }
        record_page_iterator_.Init(record_page_handler_);
        r = FetchNextRecordInPage();
        if (r != Re::RecordEof or r == Re::Success)
            return r;
    }
    next_record_.GetRecordId().slot_id = -1;
    return Re::RecordEof;
}
Re RecordFileScanner::FetchNextRecordInPage() {
    while (record_page_iterator_.HasNext()) {
        Re r = record_page_iterator_.Next(next_record_);
        if (r != Re::Success)
            return r;
        if (condition_filter_ == nullptr or condition_filter_->Filter(next_record_))
            return Re::Success;
    }
    next_record_.GetRecordId().slot_id = -1;
    return Re::RecordEof;
}
