#include "buffer_pool.h"
#include <algorithm>                 // for max
#include <assert.h>                  // for assert
#include <cstdio>                    // for printf, ssize_t
#include <errno.h>                   // for errno, EAGAIN
#include <fcntl.h>                   // for open, O_RDWR
#include <sys/stat.h>                // for timespec, S_I...
#include <sys/types.h>               // for __ssize_t
#include <time.h>                    // for clock_gettime
#include <unistd.h>                  // for close, lseek
#include <utility>                   // for pair
#include "../../common/common_defs.h"// for DebugPrint
#include "../common/re.h"            // for Re, Success
static const int32_t BP_HEADER_PAGE = 0;
static const int MEM_POOL_ITEM_NUM = 128;
unsigned long GetCurrentTime() {
    timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 1000 * 1000 * 1000UL + tp.tv_nsec;
}
int WriteN(int fd, const void *buf, int size) {
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
int ReadN(int fd, void *buf, int size) {
    char *tmp = (char *) buf;
    while (size > 0) {
        const ssize_t ret = ::read(fd, tmp, size);
        if (ret > 0) {
            tmp += ret;
            size -= ret;
            continue;
        }
        if (ret == 0)
            return -1;// end of file
        const int err = errno;
        if (EAGAIN != err && EINTR != err)
            return err;
    }
    return 0;
}
Re FrameManager::Init(int pool_num) {
    bool ret = frame_allocator_.Init(false, pool_num);
    if (ret)
        return Re::Success;
    return Re::GenericError;
}
Re FrameManager::CleanUp() {
    if (frame_lru_cache_.Count() > 0)
        return Re::GenericError;
    frame_lru_cache_.Destroy();
    return Re::Success;
}
Frame *FrameManager::GetFrame(int file_desc, int32_t page_id) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame = nullptr;
    (void) frame_lru_cache_.Get(frame_id, frame);
    return frame;
}
Frame *FrameManager::Alloc(int file_desc, int32_t page_id) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame = nullptr;
    if (frame_lru_cache_.Get(frame_id, frame))
        return nullptr;
    frame = frame_allocator_.Alloc();
    if (frame != nullptr)
        frame_lru_cache_.Put(frame_id, frame);
    return frame;
}
Re FrameManager::Free(int file_desc, int32_t page_id, Frame *frame) {
    FrameId frame_id(file_desc, page_id);
    std::lock_guard<std::mutex> lock_guard(lock_);
    Frame *frame_source = nullptr;
    bool found = frame_lru_cache_.Get(frame_id, frame_source);
    if (!found or frame != frame_source) {
        DebugPrint("DiskBufferPool:failed to find frame or got frame not match. file_desc=%d, Page_id=%d, "
                   "frame_source=%p, frame=%p\n",
                   file_desc, page_id, frame_source, frame);
        return Re::GenericError;
    }
    frame_lru_cache_.Remove(frame_id);
    frame_allocator_.Free(frame);
    return Re::Success;
}
Frame *FrameManager::GetPurgeableFrame() {
    Frame *purgeable_frame = nullptr;
    auto purge_finder = [&purgeable_frame](const FrameId &frame_id, Frame *const frame) -> bool {
        if (frame->IsPurgeable()) {
            purgeable_frame = frame;
            return false;// false to break the progress(foreachReverse)
        }
        return true;// true continue to look up
    };
    // LRU淘汰页面 (从cache中)
    frame_lru_cache_.ForeachReverse(purge_finder);
    return purgeable_frame;
}
std::list<Frame *> FrameManager::FindFrames(int file_desc) {
    std::lock_guard<std::mutex> lock_guard(lock_);
    std::list<Frame *> frames;
    auto fetcher = [&frames, file_desc](const FrameId &frame_id, Frame *const frame) -> bool {
        if (file_desc == frame_id.GetFileDesc())
            frames.push_back(frame);
        return true;
    };
    frame_lru_cache_.Foreach(fetcher);
    return frames;
}
Re DiskBufferPool::OpenFile(std::string file_name) {
    int fd = open(file_name.c_str(), O_RDWR);
    if (fd < 0) {
        DebugPrint("DiskBufferPool:failed to open file %s, because %s.\n", file_name.c_str(), strerror(errno));
        return Re::IoErrAccess;
    }
    DebugPrint("DiskBufferPool:successfully open file %s.\n", file_name.c_str());
    file_name_ = file_name, file_desc_ = fd;
    Re r = AllocateFrame(BP_HEADER_PAGE, &header_frame_);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:failed to allocate frame for header. file name %s\n", file_name_.c_str());
        close(fd);
        file_desc_ = -1;
        return r;
    }
    header_frame_->dirty_ = false;
    header_frame_->file_desc_ = fd;
    header_frame_->pin_count_ = 1;
    header_frame_->acc_time_ = GetCurrentTime();
    r = LoadPage(BP_HEADER_PAGE, header_frame_);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:failed to load first page of %s, due to %s.\n", file_name.c_str(), strerror(errno));
        header_frame_->pin_count_ = 0;
        PurgeFrame(BP_HEADER_PAGE, header_frame_);
        close(fd);
        file_desc_ = -1;
        return r;
    }
    file_header_ = reinterpret_cast<BufferPoolFileHeader *>(header_frame_->GetPageData());
    DebugPrint("DiskBufferPool:successfully open %s. file_desc=%d, hdr_frame=%p\n", file_name.c_str(), file_desc_,
               header_frame_);
    return Re::Success;
}
Re DiskBufferPool::CloseFile() {
    if (file_desc_ < 0)
        return Re::Success;// has not open file,just return success
    header_frame_->pin_count_--;
    // TODO: 理论上是在回放时回滚未提交事务，但目前没有undo log，因此不下刷数据page，只通过redo log回放
    Re r = PurgePage(0);
    if (r != Re::Success) {
        header_frame_->pin_count_++;
        DebugPrint("DiskBufferPool:failed to destroy %s, due to failed to purge all pages.\n", file_name_.c_str());
        return r;
    }
    disposed_pages_.clear();
    if (close(file_desc_) < 0) {
        DebugPrint("DiskBufferPool:failed to destroy fileId:%d, fileName:%s, error:%s\n", file_desc_,
                   file_name_.c_str(), strerror(errno));
        return Re::IoErrClose;
    }
    DebugPrint("DiskBufferPool:successfully destroy file %d:%s.\n", file_desc_, file_name_.c_str());
    file_desc_ = -1;
    return Re::Success;
}
Re DiskBufferPool::GetPage(int32_t page_id, Frame **frame) {
    // try to find out if target page was already stored in a frame (in the lru cache of frame manager)
    Frame *used_match_frame = frame_manager_.GetFrame(file_desc_, page_id);
    if (used_match_frame != nullptr) {
        // find target page in frame,just set param:frame to the frame we find and return
        used_match_frame->pin_count_++;
        used_match_frame->acc_time_ = GetCurrentTime();
        *frame = used_match_frame;
        return Re::Success;
    }
    // did not find target page,strategy is allocating a frame and put target page from file to the frame,
    // then set param:frame to the frame we allocated and return
    // todo:why not apply change to bitmap
    Frame *allocated_frame = nullptr;
    Re r = AllocateFrame(page_id, &allocated_frame);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:failed to alloc frame %s:%d, due to failed to alloc page.\n", file_name_.c_str(),
                   page_id);
        return r;
    }
    allocated_frame->dirty_ = false;
    allocated_frame->file_desc_ = file_desc_;
    allocated_frame->pin_count_ = 1;
    allocated_frame->acc_time_ = GetCurrentTime();
    // put target page's page_data from file to the allocated frame
    r = LoadPage(page_id, allocated_frame);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:failed to load page %s:%d\n", file_name_.c_str(), page_id);
        allocated_frame->pin_count_ = 0;
        PurgeFrame(page_id, allocated_frame);
        return r;
    }
    // set param:frame to the allocated page(its corresponding page is target page)
    *frame = allocated_frame;
    return Re::Success;
}
Re DiskBufferPool::AllocatePage(Frame **frame) {
    int byte, bit;
    if (file_header_->allocated_pages_num < file_header_->pages_num) {
        // load page from on-disk file
        for (int i = 0; i < file_header_->pages_num; i++) {
            byte = i / 8, bit = i % 8;
            if ((file_header_->bitmap[byte] & (1 << bit)) == 0) {
                file_header_->allocated_pages_num++;
                file_header_->bitmap[byte] |= (1 << bit);
                // TODO:do we need clean the loaded page's data?
                header_frame_->DirtyMark();
                return GetPage(i, frame);
            }
        }
    }
    if (file_header_->pages_num >= BufferPoolFileHeader::max_page_num) {
        DebugPrint("DiskBufferPool:file buffer pool is full. page count %d, max page count %d\n",
                   file_header_->pages_num, BufferPoolFileHeader::max_page_num);
        return Re::BufferPoolNoBuf;
    }
    int32_t page_id = file_header_->pages_num;
    Frame *allocated_frame = nullptr;
    Re r = AllocateFrame(page_id, &allocated_frame);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:failed to allocate frame %s, due to no free page.\n", file_name_.c_str());
        return r;
    }
    file_header_->allocated_pages_num++;
    file_header_->pages_num++;
    byte = page_id / 8, bit = page_id % 8;
    file_header_->bitmap[byte] |= (1 << bit);
    header_frame_->DirtyMark();
    allocated_frame->dirty_ = false;
    allocated_frame->file_desc_ = file_desc_;
    allocated_frame->pin_count_ = 1;
    allocated_frame->acc_time_ = GetCurrentTime();
    allocated_frame->ResetPage();
    allocated_frame->page_.page_id = file_header_->pages_num - 1;
    // Use flush operation to extension file
    r = FlushPage(*allocated_frame);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:Failed to alloc page %s , due to failed to extend one page.\n", file_name_.c_str());
        // skip return false, delay flush the extended page
        // return tmp;
    }
    *frame = allocated_frame;
    return Re::Success;
}
Re DiskBufferPool::DisposePage(int32_t page_id) {
    Re r = PurgePage(page_id);
    if (r != Re::Success) {
        DebugPrint("DiskBufferPool:Dispose page %s:%d later, due to this page is being used\n", file_name_.c_str(),
                   page_id);
        disposed_pages_.insert(page_id);
        return r;
    }
    header_frame_->dirty_ = true;
    file_header_->allocated_pages_num--;
    char tmp = 1 << (page_id % 8);
    // set corresponding bit to 0
    file_header_->bitmap[page_id / 8] &= ~tmp;
    return Re::Success;
}
Re DiskBufferPool::PurgePage(int32_t page_id) {
    Frame *used_frame = frame_manager_.GetFrame(file_desc_, page_id);
    if (used_frame != nullptr)
        return PurgeFrame(page_id, used_frame);
    return Re::Success;
}
Re DiskBufferPool::PurgeAllPages() {
    std::list<Frame *> used = frame_manager_.FindFrames(file_desc_);
    for (auto frame: used) {
        if (frame->pin_count_ > 0) {
            DebugPrint("DiskBufferPool:the page has been pinned, file_desc:%d, page id:%d, pin_count=%d\n",
                       frame->file_desc_, frame->page_.page_id, frame->pin_count_);
            continue;
        }
        if (frame->dirty_) {
            Re r = FlushPage(*frame);
            if (r != Re::Success) {
                DebugPrint("DiskBufferPool:failed to flush all pages' of %s.\n", file_name_.c_str());
                return r;
            }
        }
        frame_manager_.Free(file_desc_, frame->page_.page_id, frame);
    }
    return Re::Success;
}
Re DiskBufferPool::UnpinPage(Frame *frame) {
    assert(frame->pin_count_ >= 1);
    if (--frame->pin_count_ == 0) {
        int32_t page_num = frame->GetPageId();
        auto pages_it = disposed_pages_.find(page_num);
        if (pages_it != disposed_pages_.end()) {
            DebugPrint("DiskBufferPool:dispose file_desc:%d, page:%d\n", file_desc_, page_num);
            DisposePage(page_num);
            disposed_pages_.erase(pages_it);
        }
    }
    return Re::Success;
}
Re DiskBufferPool::GetAllocatedPagesNum(int *page_count) {
    *page_count = file_header_->allocated_pages_num;
    return Re::Success;
}
void DiskBufferPool::CheckAllPages() {
    std::list<Frame *> frames = frame_manager_.FindFrames(file_desc_);
    for (auto &frame: frames)
        frame->Desc();
}
Re DiskBufferPool::CheckAllPagesUnpinned() {
    std::list<Frame *> frames = frame_manager_.FindFrames(file_desc_);
    for (auto &frame: frames) {
        if (frame->GetPageId() == BP_HEADER_PAGE && frame->pin_count_ > 1)
            DebugPrint("DiskBufferPool:this page has been pinned. file desc=%d, page id:%d, pin count=%d\n", file_desc_,
                       frame->GetPageId(), frame->pin_count_);
        else if (frame->GetPageId() != BP_HEADER_PAGE && frame->pin_count_ > 0)
            DebugPrint("DiskBufferPool:this page has been pinned. file desc=%d, page id:%d, pin count=%d\n", file_desc_,
                       frame->GetPageId(), frame->pin_count_);
    }
    DebugPrint("DiskBufferPool:all pages have been checked of file desc %d\n", file_desc_);
    return Re::Success;
}
int DiskBufferPool::GetFileDesc() const {
    return file_desc_;
}
Re DiskBufferPool::FlushPage(Frame &frame) {
    DebugPrint("DiskBufferPool:flush page page_id:%d\n", frame.GetPageId());
    Page &page = frame.page_;
    long long offset = ((long long) page.page_id) * sizeof(Page);
    if (lseek(file_desc_, offset, SEEK_SET) == offset - 1) {
        DebugPrint("DiskBufferPool:Failed to flush page %lld of %d due to failed to seek %s.\n", offset, file_desc_,
                   strerror(errno));
        return Re::IoErrSeek;
    }
    if (WriteN(file_desc_, &page, sizeof(Page)) != 0) {
        DebugPrint("DiskBufferPool:Failed to flush page %lld of %d due to %s.\n", offset, file_desc_, strerror(errno));
        return Re::IoErrWrite;
    }
    frame.dirty_ = false;
    DebugPrint("DiskBufferPool:Flush block. file desc=%d, page id=%d\n", file_desc_, page.page_id);
    return Re::Success;
}
Re DiskBufferPool::FlushAllPages() {
    std::list<Frame *> used = frame_manager_.FindFrames(file_desc_);
    for (Frame *frame: used) {
        Re r = FlushPage(*frame);
        if (r != Re::Success) {
            DebugPrint("DiskBufferPool:failed to flush all pages\n");
            return r;
        }
    }
    return Re::Success;
}
Re DiskBufferPool::RecoverPage(int32_t page_id) {
    int byte = page_id / 8, bit = page_id % 8;
    if (!(file_header_->bitmap[byte] & (1 << bit))) {
        file_header_->bitmap[byte] |= (1 << bit);
        file_header_->allocated_pages_num++;
        file_header_->pages_num++;
        header_frame_->DirtyMark();
    }
    return Re::Success;
}
Re DiskBufferPool::AllocateFrame(int32_t page_id, Frame **buf) {
    while (true) {
        Frame *frame = frame_manager_.Alloc(file_desc_, page_id);
        if (frame != nullptr) {
            *buf = frame;
            return Re::Success;
        }
        frame = frame_manager_.GetPurgeableFrame();
        if (frame == nullptr) {
            DebugPrint("DiskBufferPool:all pages have been used and pinned.\n");
            return Re::NoMem;
        }
        if (frame->dirty_) {
            Re r = buffer_pool_manager_.FlushPage(*frame);
            if (r != Re::Success) {
                DebugPrint("DiskBufferPool:failed to allocate block due to failed to flush old block.\n");
                return r;
            }
        }
        // NOTE:after free,we can get frame next loop
        frame_manager_.Free(frame->GetFileDesc(), frame->GetPageId(), frame);
    }
}
Re DiskBufferPool::PurgeFrame(int32_t page_id, Frame *used_frame) {
    if (used_frame->pin_count_ > 0) {
        DebugPrint("DiskBufferPool:begin to free page %d of %d(file id), but it's pinned, pin_count:%d.\n",
                   used_frame->GetPageId(), used_frame->file_desc_, used_frame->pin_count_);
        return Re::LockedUnlock;
    }
    if (used_frame->dirty_) {
        Re r = FlushPage(*used_frame);
        if (r != Re::Success) {
            DebugPrint("DiskBufferPool:failed to flush page %d of %d(file desc) during purge page.\n",
                       used_frame->GetPageId(), used_frame->file_desc_);
            return r;
        }
    }
    DebugPrint("DiskBufferPool:successfully purge frame =%p, page %d of %d(file desc)\n", used_frame,
               used_frame->GetPageId(), used_frame->file_desc_);
    frame_manager_.Free(file_desc_, page_id, used_frame);
    return Re::Success;
}
Re DiskBufferPool::CheckPageId(int32_t page_id) {
    if (page_id >= file_header_->pages_num) {
        DebugPrint("DiskBufferPool:invalid page_id:%d, file's name:%s\n", page_id, file_name_.c_str());
        return Re::BufferPoolInvalidPageId;
    }
    // page in buffer must be set in bitmap
    if ((file_header_->bitmap[page_id / 8] & (1 << (page_id % 8))) == 0) {
        DebugPrint("DiskBufferPool:invalid page_id:%d, file's name:%s\n", page_id, file_name_.c_str());
        return Re::BufferPoolInvalidPageId;
    }
    return Re::Success;
}
Re DiskBufferPool::LoadPage(int32_t page_id, Frame *frame) {
    long long offset = ((long long) page_id) * sizeof(Page);
    if (lseek(file_desc_, offset, SEEK_SET) == -1) {
        DebugPrint("DiskBufferPool:failed to load page %s:%d, due to failed to lseek:%s.\n", file_name_.c_str(),
                   page_id, strerror(errno));
        return Re::IoErrSeek;
    }
    int ret = ReadN(file_desc_, &(frame->page_), sizeof(Page));
    if (ret != 0) {
        DebugPrint("DiskBufferPool:failed to load page %s:%d, due to failed to read data:%s, ret=%d, page count=%d\n",
                   file_name_.c_str(), page_id, strerror(errno), ret, file_header_->allocated_pages_num);
        return Re::IoErrRead;
    }
    return Re::Success;
}
void DiskBufferPool::Destroy() {
    if (file_desc_ < 0)
        return;// has not open file,just return success
    FlushAllPages();
    header_frame_->pin_count_--;
    // // TODO: 理论上是在回放时回滚未提交事务，但目前没有undo log，因此不下刷数据page，只通过redo log回放
    Re r = PurgePage(0);
    if (r != Re::Success) {
        header_frame_->pin_count_++;
        DebugPrint("DiskBufferPool:failed to destroy %s, due to failed to purge all pages.\n", file_name_.c_str());
        return;
    }
    disposed_pages_.clear();
    if (close(file_desc_) < 0) {
        DebugPrint("DiskBufferPool:failed to destroy fileId:%d, fileName:%s, error:%s\n", file_desc_,
                   file_name_.c_str(), strerror(errno));
        return;
    }
    DebugPrint("DiskBufferPool:successfully destroy file %d:%s.\n", file_desc_, file_name_.c_str());
    file_desc_ = -1;
    DebugPrint("DiskBufferPool:disk buffer pool destructed\n");
}
void GlobalBufferPoolManager::Init() {
    frame_manager_.Init(MEM_POOL_ITEM_NUM);
    DebugPrint("GlobalBufferPoolManager:initialized done\n");
}
Re GlobalBufferPoolManager::CreateFile(const char *file_name) {
    int fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
    if (fd < 0) {
        DebugPrint("GlobalBufferPoolManager:failed to createFilter %s, due to %s.\n", file_name, strerror(errno));
        return Re::SchemaDbExist;
    }
    close(fd);
    fd = open(file_name, O_RDWR);
    if (fd < 0) {
        DebugPrint("GlobalBufferPoolManager:failed to open for readwrite %s, due to %s.\n", file_name, strerror(errno));
        return Re::IoErrAccess;
    }
    Page page;
    memset(&page, 0, sizeof(Page));
    // use page.page_data to represent the file header,
    // i.e. &page.page_data[0] equal to &file_header.pages_num,
    //&page_data[4] equal to &file_header.allocated_pages_num,
    //&page_data[8] equal to &file_header.bitmap and left space of page.page_data was part of array file_header.bitmap
    BufferPoolFileHeader *file_header = reinterpret_cast<BufferPoolFileHeader *>(page.page_data);
    file_header->pages_num = 1, file_header->allocated_pages_num = 1;
    char *bitmap = file_header->bitmap;
    // set the first bit of the char(byte) to 1(NOTE:i.e. the first page was used(the file header page itself)
    bitmap[0] |= 0x01;
    if (lseek(fd, 0, SEEK_SET) == -1) {
        DebugPrint("GlobalBufferPoolManager:failed to seek file %s to position 0, due to %s .\n", file_name,
                   strerror(errno));
        close(fd);
        return Re::IoErrSeek;
    }
    if (WriteN(fd, (char *) &page, sizeof(Page)) != 0) {
        DebugPrint("GlobalBufferPoolManager:failed to write header to file %s, due to %s.\n", file_name,
                   strerror(errno));
        close(fd);
        return Re::IoErrWrite;
    }
    close(fd);
    DebugPrint("GlobalBufferPoolManager:successfully create file %s.\n", file_name);
    return Re::Success;
}
Re GlobalBufferPoolManager::OpenFile(std::string file_name, DiskBufferPool *&bp) {
    std::string x(file_name);
    if (buffer_pools_.find(x) != buffer_pools_.end()) {
        DebugPrint("GlobalBufferPoolManager:file already opened and has a buffer now. file name=%s\n",
                   file_name.c_str());
        return Re::BufferPoolOpen;
    }
    auto new_buffer_pool = new DiskBufferPool(*this, frame_manager_);
    Re r = new_buffer_pool->OpenFile(file_name);
    if (r != Re::Success) {
        DebugPrint("GlobalBufferPoolManager:failed to open file name\n");
        new_buffer_pool->Destroy();
        return r;
    }
    buffer_pools_.insert(std::pair<std::string, DiskBufferPool *>(x, new_buffer_pool));
    fd_buffer_pools_.insert(std::pair<int, DiskBufferPool *>(new_buffer_pool->GetFileDesc(), new_buffer_pool));
    bp = new_buffer_pool;
    return Re::Success;
}
Re GlobalBufferPoolManager::CloseFile(std::string file_name) {
    auto iter = buffer_pools_.find(file_name);
    if (iter == buffer_pools_.end()) {
        DebugPrint("GlobalBufferPoolManager:file has not opened: %s\n", file_name.c_str());
        return Re::Internal;
    }
    int fd = iter->second->GetFileDesc();
    fd_buffer_pools_.erase(fd);
    buffer_pools_.erase(iter);
    DiskBufferPool *bp = iter->second;
    bp->CloseFile();
    return Re::Success;
}
Re GlobalBufferPoolManager::FlushPage(Frame &frame) {
    int fd = frame.GetFileDesc();
    auto iter = fd_buffer_pools_.find(fd);
    if (iter == fd_buffer_pools_.end()) {
        DebugPrint("GlobalBufferPoolManager:unknown buffer pool of fd %d\n", fd);
        return Re::Internal;
    }
    DiskBufferPool *bp = iter->second;
    return bp->FlushPage(frame);
}
void GlobalBufferPoolManager::Destroy() {
    std::unordered_map<std::string, DiskBufferPool *> tmp_bps;
    tmp_bps.swap(buffer_pools_);
    for (auto &iter: tmp_bps)
        iter.second->Destroy();
}
Re BufferPoolIterator::Init(DiskBufferPool &bp, int32_t start_page /* = 0 */) {
    bit_map_.Init(bp.file_header_->bitmap, bp.file_header_->pages_num);
    current_page_id_ = std::max(0, start_page);
    return Re::Success;
}
bool BufferPoolIterator::HasNext() {
    return bit_map_.NextSetBit(current_page_id_ + 1) != -1;
}
int32_t BufferPoolIterator::Next() {
    int32_t next_page = bit_map_.NextSetBit(current_page_id_ + 1);
    if (next_page != -1)
        current_page_id_ = next_page;
    return next_page;
}
Re BufferPoolIterator::Reset() {
    current_page_id_ = 0;
    return Re::Success;
}
void Frame::Desc() {
    printf("Frame: dirty_mark:%d pin_count:%d file_desc:%d page_id:%d\n", dirty_, pin_count_, file_desc_,
           page_.page_id);
}
