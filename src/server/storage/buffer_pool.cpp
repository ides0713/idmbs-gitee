#include "buffer_pool.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static const int32_t BP_HEADER_PAGE = 0;
static const int MEM_POOL_ITEM_NUM = 128;

unsigned long getCurrentTime() {
    timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 1000 * 1000 * 1000UL + tp.tv_nsec;
}

int writeN(int fd, const void *buf, int size) {
    const char *tmp = (const char *) buf;
    while (size > 0) {
        const __ssize_t ret = write(fd, tmp, size);
        if (ret >= 0) {
            tmp += ret;
            size -= ret;
            continue;
        }
        const int err = errno;
        if (EAGAIN != err && EINTR != err)
            return err;
    }
    return 0;
}

int readN(int fd, void *buf, int size) {
    char *tmp = (char *) buf;
    while (size > 0) {
        const ssize_t ret = ::read(fd, tmp, size);
        if (ret > 0) {
            tmp += ret;
            size -= ret;
            continue;
        }
        if (0 == ret)
            return -1; // end of file

        const int err = errno;
        if (EAGAIN != err && EINTR != err)
            return err;
    }
    return 0;
}

Re FrameManager::initialize(int pool_num) {
    bool ret = memory_pool_allocator_.initialize(false, pool_num);
    if (ret)
        return Re::Success;
    return Re::GenericError;
}

Re FrameManager::cleanUp() {
    if (frames_lru_cache_.count() > 0)
        return Re::GenericError;
    frames_lru_cache_.destroy();
    return Re::Success;
}

Frame *FrameManager::get(int file_desc, int32_t page_id) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame = nullptr;
    ///@brief 显明丢弃该值
    (void) frames_lru_cache_.get(frame_id, frame);
    return frame;
}

Frame *FrameManager::alloc(int file_desc, int32_t page_id) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame = nullptr;
    bool found = frames_lru_cache_.get(frame_id, frame);
    if (found)
        return nullptr;
    frame = memory_pool_allocator_.alloc();
    if (frame != nullptr)
        frames_lru_cache_.put(frame_id, frame);
    return frame;
}

Re FrameManager::free(int file_desc, int32_t page_id, Frame *frame) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame_source = nullptr;
    bool found = frames_lru_cache_.get(frame_id, frame_source);
    if (!found or frame != frame_source) {
        debugPrint(
                "DiskBufferPool:failed to find frame or got frame not match. file_desc=%d, PageNum=%d, frame_source=%p, frame=%p",
                file_desc, page_id, frame_source, frame);
        return Re::GenericError;
    }
    frames_lru_cache_.remove(frame_id);
    memory_pool_allocator_.free(frame);
    return Re::Success;
}

Frame *FrameManager::beginPurge() {
    Frame *p_frame = nullptr;
    auto purge_finder = [&p_frame](const FrameId &frame_id, Frame *const frame) -> bool {
        if (frame->isPurgable()) {
            p_frame = frame;
            return false; // false to break the progress
        }
        return true; // true continue to look up
    };
    ///@brief LRU淘汰页面 (从cache中)
    frames_lru_cache_.foreachReverse(purge_finder);
    return p_frame;
}

std::list<Frame *> FrameManager::findList(int file_desc) {
    std::lock_guard<std::mutex> lock_guard(lock_);
    std::list<Frame *> frames;
    auto fetcher = [&frames, file_desc](const FrameId &frame_id, Frame *const frame) -> bool {
        if (file_desc == frame_id.getFileDesc())
            frames.push_back(frame);
        return true;
    };
    frames_lru_cache_.foreach(fetcher);
    return frames;
}

DiskBufferPool::~DiskBufferPool() {
    closeFile();
    debugPrint("DiskBufferPool:Exit\n");
}

Re DiskBufferPool::openFile(const char *file_name) {
    int fd;
    if ((fd = open(file_name, O_RDWR)) < 0) {
        debugPrint("DiskBufferPool:Failed to open file %s, because %s.\n", file_name, strerror(errno));
        return Re::IoErrAccess;
    }
    debugPrint("DiskBufferPool:Successfully open file %s.\n", file_name);
    file_name_ = file_name;
    file_desc_ = fd;
    Re r = allocateFrame(BP_HEADER_PAGE, &header_frame_);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:failed to allocate frame for header. file name %s\n", file_name_.c_str());
        close(fd);
        file_desc_ = -1;
        return r;
    }
    header_frame_->dirty_ = false;
    header_frame_->file_desc_ = fd;
    header_frame_->pin_count_ = 1;
    header_frame_->acc_time_ = getCurrentTime();
    r = loadPage(BP_HEADER_PAGE, header_frame_);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Failed to load first page of %s, due to %s.\n", file_name, strerror(errno));
        header_frame_->pin_count_ = 0;
        purgeFrame(BP_HEADER_PAGE, header_frame_);
        close(fd);
        file_desc_ = -1;
        return r;
    }
    file_header_ = (BufferPoolFileHeader *) header_frame_->getPageData();
    debugPrint("DiskBufferPool:Successfully open %s. file_desc=%d, hdr_frame=%p\n", file_name, file_desc_,
               header_frame_);
    return Re::Success;
}

Re DiskBufferPool::closeFile() {
    if (file_desc_ < 0)
        return Re::Success;
    header_frame_->pin_count_--;
    // TODO: 理论上是在回放时回滚未提交事务，但目前没有undo log，因此不下刷数据page，只通过redo log回放
    Re r = purgePage(0);
    if (r != Re::Success) {
        header_frame_->pin_count_++;
        debugPrint("DiskBufferPool:Failed to close %s, due to failed to purge all pages.\n", file_name_.c_str());
        return r;
    }
    disposed_pages_.clear();
    if (close(file_desc_) < 0) {
        debugPrint("DiskBufferPool:Failed to close fileId:%d, fileName:%s, error:%s\n", file_desc_, file_name_.c_str(),
                   strerror(errno));
        return Re::IoErrClose;
    }
    debugPrint("DiskBufferPool:Successfully close file %d:%s.\n", file_desc_, file_name_.c_str());
    file_desc_ = -1;

    bp_manager_.closeFile(file_name_.c_str());
    return Re::Success;
}

Re DiskBufferPool::getThisPage(int32_t page_id, Frame **frame) {
    Frame *used_match_frame = frame_manager_.get(file_desc_, page_id);
    if (used_match_frame != nullptr) {
        used_match_frame->pin_count_++;
        used_match_frame->acc_time_ = getCurrentTime();
        *frame = used_match_frame;
        return Re::Success;
    }
    // Allocate one page and load the data into this page
    Frame *allocated_frame = nullptr;
    Re r = allocateFrame(page_id, &allocated_frame);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Failed to alloc frame %s:%d, due to failed to alloc page.\n", file_name_.c_str(),
                   page_id);
        return r;
    }
    allocated_frame->dirty_ = false;
    allocated_frame->file_desc_ = file_desc_;
    allocated_frame->pin_count_ = 1;
    allocated_frame->acc_time_ = getCurrentTime();
    r = loadPage(page_id, allocated_frame);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Failed to load page %s:%d\n", file_name_.c_str(), page_id);
        allocated_frame->pin_count_ = 0;
        purgeFrame(page_id, allocated_frame);
        return r;
    }
    *frame = allocated_frame;
    return Re::Success;
}

Re DiskBufferPool::allocatePage(Frame **frame) {
    int byte = 0, bit = 0;
    if ((file_header_->allocated_pages) < (file_header_->page_count)) {
        for (int i = 0; i < file_header_->page_count; i++) {
            byte = i / 8;
            bit = i % 8;
            if (((file_header_->bitmap[byte]) & (1 << bit)) == 0) {
                (file_header_->allocated_pages)++;
                file_header_->bitmap[byte] |= (1 << bit);
                // TODO,  do we need clean the loaded page's data?
                header_frame_->dirtyMark();
                return getThisPage(i, frame);
            }
        }
    }

    if (file_header_->page_count >= BufferPoolFileHeader::max_page_num) {
        debugPrint("DiskBufferPool:file buffer pool is full. page count %d, max page count %d\n",
                   file_header_->page_count, BufferPoolFileHeader::max_page_num);
        return Re::BufferPoolNoBuf;
    }
    int32_t page_id = file_header_->page_count;
    Frame *allocated_frame = nullptr;
    Re r = allocateFrame(page_id, &allocated_frame);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Failed to allocate frame %s, due to no free page.\n", file_name_.c_str());
        return r;
    }

    file_header_->allocated_pages++;
    file_header_->page_count++;

    byte = page_id / 8;
    bit = page_id % 8;
    file_header_->bitmap[byte] |= (1 << bit);
    header_frame_->dirtyMark();

    allocated_frame->dirty_ = false;
    allocated_frame->file_desc_ = file_desc_;
    allocated_frame->pin_count_ = 1;
    allocated_frame->acc_time_ = getCurrentTime();
    allocated_frame->resetPage();
    allocated_frame->page_.page_id = file_header_->page_count - 1;

    // Use flush operation to extension file
    r = flushPage(*allocated_frame);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Failed to alloc page %s , due to failed to extend one page.\n", file_name_.c_str());
        // skip return false, delay flush the extended page
        // return tmp;
    }
    *frame = allocated_frame;
    return Re::Success;
}

Re DiskBufferPool::disposePage(int32_t page_id) {
    Re r = purgePage(page_id);
    if (r != Re::Success) {
        debugPrint("DiskBufferPool:Dispose page %s:%d later, due to this page is being used\n", file_name_.c_str(),
                   page_id);
        disposed_pages_.insert(page_id);
        return r;
    }
    header_frame_->dirty_ = true;
    file_header_->allocated_pages--;
    char tmp = 1 << (page_id % 8);
    file_header_->bitmap[page_id / 8] &= ~tmp;
    return Re::Success;
}

Re DiskBufferPool::purgePage(int32_t page_num) {
    Frame *used_frame = frame_manager_.get(file_desc_, page_num);
    if (used_frame != nullptr)
        return purgeFrame(page_num, used_frame);
    return Re::Success;
}

Re DiskBufferPool::purgeAllPages() {
    std::list<Frame *> used = frame_manager_.findList(file_desc_);
    for (auto frame: used) {
        if (frame->pin_count_ > 0) {
            debugPrint("DiskBufferPool:The page has been pinned, file_desc:%d, page id:%d, pin_count=%d\n",
                       frame->file_desc_, frame->page_.page_id, frame->pin_count_);
            continue;
        }
        if (frame->dirty_) {
            Re r = flushPage(*frame);
            if (r != Re::Success) {
                debugPrint("DiskBufferPool:Failed to flush all pages' of %s.\n", file_name_.c_str());
                return r;
            }
        }
        frame_manager_.free(file_desc_, frame->page_.page_id, frame);
    }
    return Re::Success;
}

Re DiskBufferPool::unpinPage(Frame *frame) {
    assert(frame->pin_count_ >= 1);
    if (--frame->pin_count_ == 0) {
        int32_t page_num = frame->getPageId();
        auto pages_it = disposed_pages_.find(page_num);
        if (pages_it != disposed_pages_.end()) {
            debugPrint("DiskBufferPool:Dispose file_desc:%d, page:%d\n", file_desc_, page_num);
            disposePage(page_num);
            disposed_pages_.erase(pages_it);
        }
    }
    return Re::Success;
}

Re DiskBufferPool::getPageCount(int *page_count) {
    *page_count = file_header_->allocated_pages;
    return Re::Success;
}

Re DiskBufferPool::checkAllPagesUnpinned() {
    std::list<Frame *> frames = frame_manager_.findList(file_desc_);
    for (auto &frame: frames) {
        if (frame->getPageId() == BP_HEADER_PAGE && frame->pin_count_ > 1)
            debugPrint("DiskBufferPool:This page has been pinned. file desc=%d, page num:%d, pin count=%d\n",
                       file_desc_, frame->getPageId(), frame->pin_count_);
        else if (frame->getPageId() != BP_HEADER_PAGE && frame->pin_count_ > 0)
            debugPrint("DiskBufferPool:This page has been pinned. file desc=%d, page num:%d, pin count=%d\n",
                       file_desc_, frame->getPageId(), frame->pin_count_);
    }
    debugPrint("DiskBufferPool:all pages have been checked of file desc %d\n", file_desc_);
    return Re::Success;
}

int DiskBufferPool::getFileDesc() const {
    return file_desc_;
}

Re DiskBufferPool::flushPage(Frame &frame) {
    Page &page = frame.page_;
    long long offset = ((long long) page.page_id) * sizeof(Page);
    if (lseek(file_desc_, offset, SEEK_SET) == offset - 1) {
        debugPrint("DiskBufferPool:Failed to flush page %lld of %d due to failed to seek %s.\n", offset, file_desc_,
                   strerror(errno));
        return Re::IoErrSeek;
    }
    if (writeN(file_desc_, &page, sizeof(Page)) != 0) {
        debugPrint("DiskBufferPool:Failed to flush page %lld of %d due to %s.\n", offset, file_desc_, strerror(errno));
        return Re::IoErrWrite;
    }
    frame.dirty_ = false;
    debugPrint("DiskBufferPool:Flush block. file desc=%d, page num=%d", file_desc_, page.page_id);
    return Re::Success;
}

Re DiskBufferPool::flushAllPages() {
    std::list<Frame *> used = frame_manager_.findList(file_desc_);
    for (Frame *frame: used) {
        Re r = flushPage(*frame);
        if (r != Re::Success) {
            debugPrint("DiskBufferPool:failed to flush all pages\n");
            return r;
        }
    }
    return Re::Success;
}

Re DiskBufferPool::recoverPage(int32_t page_id) {
    int byte = 0, bit = 0;
    byte = page_id / 8;
    bit = page_id % 8;
    if (!(file_header_->bitmap[byte] & (1 << bit))) {
        file_header_->bitmap[byte] |= (1 << bit);
        file_header_->allocated_pages++;
        file_header_->page_count++;
        header_frame_->dirtyMark();
    }
    return Re::Success;
}

Re DiskBufferPool::allocateFrame(int32_t page_id, Frame **buf) {
    while (true) {
        Frame *frame = frame_manager_.alloc(file_desc_, page_id);
        if (frame != nullptr) {
            *buf = frame;
            return Re::Success;
        }
        frame = frame_manager_.beginPurge();
        if (frame == nullptr) {
            debugPrint("DiskBufferPool:all pages have been used and pinned.\n");
            return Re::NoMem;
        }
        if (frame->dirty_) {
            Re r = bp_manager_.flushPage(*frame);
            if (r != Re::Success) {
                debugPrint("DiskBufferPool:failed to allocate block due to failed to flush old block.\n");
                return r;
            }
        }
        frame_manager_.free(frame->getFileDesc(), frame->getPageId(), frame);
    }
    return Re::Internal;
}

Re DiskBufferPool::purgeFrame(int32_t page_id, Frame *used_frame) {
    if (used_frame->pin_count_ > 0) {
        debugPrint("DiskBufferPool:begin to free page %d of %d(file id), but it's pinned, pin_count:%d.\n",
                   used_frame->getPageId(), used_frame->file_desc_, used_frame->pin_count_);
        return Re::LockedUnlock;
    }
    if (used_frame->dirty_) {
        Re r = flushPage(*used_frame);
        if (r != Re::Success) {
            debugPrint("DiskBufferPool:failed to flush page %d of %d(file desc) during purge page.\n",
                       used_frame->getPageId(),
                       used_frame->file_desc_);
            return r;
        }
    }
    debugPrint("DiskBufferPool:successfully purge frame =%p, page %d of %d(file desc)\n", used_frame,
               used_frame->getPageId(),
               used_frame->file_desc_);
    frame_manager_.free(file_desc_, page_id, used_frame);
    return Re::Success;
}

Re DiskBufferPool::checkPageId(int32_t page_id) {
    if (page_id >= file_header_->page_count) {
        debugPrint("DiskBufferPool:invalid pageNum:%d, file's name:%s\n", page_id, file_name_.c_str());
        return Re::BufferPoolInvalidPageId;
    }
    if ((file_header_->bitmap[page_id / 8] & (1 << (page_id % 8))) == 0) {
        debugPrint("DiskBufferPool:invalid pageNum:%d, file's name:%s\n", page_id, file_name_.c_str());
        return Re::BufferPoolInvalidPageId;
    }
    return Re::Success;
}

Re DiskBufferPool::loadPage(int32_t page_id, Frame *frame) {
    long long offset = ((long long) page_id) * sizeof(Page);
    if (lseek(file_desc_, offset, SEEK_SET) == -1) {
        debugPrint("DiskBufferPool:failed to load page %s:%d, due to failed to lseek:%s.\n",
                   file_name_.c_str(), page_id, strerror(errno));

        return Re::IoErrSeek;
    }
    int ret = readN(file_desc_, &(frame->page_), sizeof(Page));
    if (ret != 0) {
        debugPrint("DiskBufferPool:failed to load page %s:%d, due to failed to read data:%s, ret=%d, page count=%d",
                   file_name_.c_str(), page_id, strerror(errno), ret, file_header_->allocated_pages);
        return Re::IoErrRead;
    }
    return Re::Success;
}

Re BufferPoolIterator::init(DiskBufferPool &bp, int32_t start_page /* = 0 */) {
    bit_map_.init(bp.file_header_->bitmap, bp.file_header_->page_count);
    if (start_page <= 0)
        current_page_num_ = 0;
    else
        current_page_num_ = start_page;
    return Re::Success;
}

bool BufferPoolIterator::hasNext() {
    return bit_map_.nextSetBit(current_page_num_ + 1) != -1;
}

int32_t BufferPoolIterator::next() {
    int32_t next_page = bit_map_.nextSetBit(current_page_num_ + 1);
    if (next_page != -1)
        current_page_num_ = next_page;
    return next_page;
}

Re BufferPoolIterator::reset() {
    current_page_num_ = 0;
    return Re::Success;
}


void GlobalBufferPoolManager::initialize() {
    frame_manager_.initialize(MEM_POOL_ITEM_NUM);
    debugPrint("GlobalBufferPoolManager:initialized done\n");
}

Re GlobalBufferPoolManager::createFile(const char *file_name) {
    int fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
    if (fd < 0) {
        debugPrint("GlobalBufferPoolManager:failed to create %s, due to %s.\n", file_name, strerror(errno));
        return Re::SchemaDbExist;
    }
    close(fd);
    fd = open(file_name, O_RDWR);
    if (fd < 0) {
        debugPrint("GlobalBufferPoolManager:failed to open for readwrite %s, due to %s.\n", file_name, strerror(errno));
        return Re::IoErrAccess;
    }
    Page page;
    memset(&page, 0, sizeof(Page));
    BufferPoolFileHeader *file_header = (BufferPoolFileHeader *) page.page_data;
    file_header->allocated_pages = 1;
    file_header->page_count = 1;
    char *bitmap = file_header->bitmap;
    bitmap[0] |= 0x01;
    if (lseek(fd, 0, SEEK_SET) == -1) {
        debugPrint("GlobalBufferPoolManager:failed to seek file %s to position 0, due to %s .\n", file_name,
                   strerror(errno));
        close(fd);
        return Re::IoErrSeek;
    }
    if (writeN(fd, (char *) &page, sizeof(Page)) != 0) {
        debugPrint("GlobalBufferPoolManager:failed to write header to file %s, due to %s.\n", file_name,
                   strerror(errno));
        close(fd);
        return Re::IoErrWrite;
    }
    close(fd);
    debugPrint("GlobalBufferPoolManager:successfully create %s.\n", file_name);
    return Re::Success;
}

Re GlobalBufferPoolManager::openFile(const char *file_name, DiskBufferPool *&bp) {
    std::string x(file_name);
    if (buffer_pools_.find(x) != buffer_pools_.end()) {
        debugPrint("GlobalBufferPoolManager:file already opened. file name=%s\n", file_name);
        return Re::BufferPoolOpen;
    }
    DiskBufferPool *buffer_pool = new DiskBufferPool(*this, frame_manager_);
    Re r = buffer_pool->openFile(file_name);
    if (r != Re::Success) {
        debugPrint("GlobalBufferPoolManager:failed to open file name\n");
        delete buffer_pool;
        return r;
    }
    buffer_pools_.insert(std::pair<std::string, DiskBufferPool *>(x, buffer_pool));
    fd_buffer_pools_.insert(std::pair<int, DiskBufferPool *>(buffer_pool->getFileDesc(), buffer_pool));
    bp = buffer_pool;
    return Re::Success;
}

Re GlobalBufferPoolManager::closeFile(const char *file_name) {
    std::string f_name(file_name);
    auto iter = buffer_pools_.find(f_name);
    if (iter == buffer_pools_.end()) {
        debugPrint("GlobalBufferPoolManager:file has not opened: %s\n", file_name);
        return Re::Internal;
    }
    int fd = iter->second->getFileDesc();
    fd_buffer_pools_.erase(fd);
    DiskBufferPool *bp = iter->second;
    buffer_pools_.erase(iter);
    delete bp;
    return Re::Success;
}

Re GlobalBufferPoolManager::flushPage(Frame &frame) {
    int fd = frame.getFileDesc();
    auto iter = fd_buffer_pools_.find(fd);
    if (iter == fd_buffer_pools_.end()) {
        debugPrint("GlobalBufferPoolManager:unknown buffer pool of fd %d\n", fd);
        return Re::Internal;
    }
    DiskBufferPool *bp = iter->second;
    return bp->flushPage(frame);
}

void GlobalBufferPoolManager::destroy() {
    std::unordered_map<std::string, DiskBufferPool *> tmp_bps;
    tmp_bps.swap(buffer_pools_);
    for (auto &iter: tmp_bps)
        delete iter.second;
}
