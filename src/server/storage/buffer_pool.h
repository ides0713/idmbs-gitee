#pragma once

#include <stdint.h>
#include "LRU_cache.h"
#include "mem_pool.h"
#include "bitmap.h"
#include <string.h>
#include <unordered_map>
#include "../common/server_defs.h"

#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 14)
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(int32_t))
#define BP_FILE_HDR_SIZE (sizeof(BPFileHeader))

class Page {
public:
private:
    int32_t page_id_;
    char page_data_[BP_PAGE_SIZE];

private:
    friend class Frame;

    friend class DiskBufferPool;

    friend class BufferPoolManager;
};

// first page of bpfile
struct BPFileHeader {
    int32_t page_count;      //! 当前文件一共有多少个页面
    int32_t allocated_pages; //! 已经分配了多少个页面
    // 指向数据后一位 (新鲜的用法)
    char bitmap[0]; //! 页面分配位图, 第0个页面(就是当前页面)，总是1

    /**
     * 能够分配的最大的页面个数，即bitmap的字节数 乘以8
     */
    // file header是一页 使用该页存储 字节*8个 bit
    static const int MAX_PAGE_NUM = (BP_PAGE_DATA_SIZE - sizeof(page_count) - sizeof(allocated_pages)) * 8;
};

class Frame {
public:
    Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1) {}

    void resetPage() { memset(&page_, 0, sizeof(page_)); }

    void dirtyMark() { dirty_ = true; };

    int32_t getPageId() const { return page_.page_id_; };

    void setPageId(int32_t id) { page_.page_id_ = id; }

    char *getPageData() { return page_.page_data_; };

    // void setPageData(const char * data);
    int getFileDesc() const { return file_desc_; };

    void setFileDesc(int file_desc) { file_desc_ = file_desc; }

    bool isPurgable() { return pin_count_ <= 0; }

private:
    bool dirty_;
    unsigned int pin_count_;
    unsigned long acc_time_;
    int file_desc_;
    Page page_;

private:
    friend class DiskBufferPool;
};

///@brief get frameid by hash file_desc and page_num
class FrameId {
public:
    FrameId(int file_desc, int32_t page_num) : file_desc_(file_desc), page_id_(page_num) {}

    bool equal_to(const FrameId &other) const { return file_desc_ == other.file_desc_ && page_id_ == other.page_id_; }

    bool operator==(const FrameId &other) const { return this->equal_to(other); }

    size_t hash() const { return static_cast<size_t>(file_desc_) << 32L | page_id_; }

    int getFileDesc() const { return file_desc_; }

    int32_t getPageId() const { return page_id_; }

private:
    int file_desc_;
    int32_t page_id_;
};

class FrameManager {
public:
    FrameManager(const char *tag) : memory_pool_allocator_(tag) {}

    RE initialize(int pool_num);

    RE cleanUp();

    Frame *get(int file_desc, int32_t page_num);

    Frame *alloc(int file_desc, int32_t page_num);

    RE free(int file_desc, int32_t page_num, Frame *frame);

    Frame *beginPurge();

    std::list<Frame *> findList(int file_desc);

    size_t getFrameNum() const { return frames_LRU_cache_.count(); }

    size_t getTotalFrameNum() const { return memory_pool_allocator_.getSize(); }

private:
    class FrameIdHasher {
    public:
        size_t operator()(const FrameId &frame_id) const { return frame_id.hash(); }

    private:
    };

private:
    std::mutex lock_;
    LRUCache<FrameId, Frame *, FrameIdHasher, std::equal_to<FrameId>> frames_LRU_cache_;
    MemoryPool<Frame> memory_pool_allocator_;
};

class BufferPoolManager;
///@brief class above does not make on-disk change

class DiskBufferPool {
public:
    DiskBufferPool(BufferPoolManager &bp_manager, FrameManager &frame_manager) : bp_manager_(bp_manager),
                                                                                 frame_manager_(frame_manager) {}

    ~DiskBufferPool();

    // RE create_file(const char *file_name);
    RE openFile(const char *file_name);

    RE closeFile();

    RE getThisPage(int32_t page_num, Frame **frame);

    RE allocatePage(Frame **frame);

    RE disposePage(int32_t page_num);

    RE purgePage(int32_t page_num);

    RE purgeAllPages();

    RE unpinPage(Frame *frame);

    RE getPageCount(int *page_count);

    RE checkAllPagesUnpinned();

    int getFileDesc() const;

    RE flushPage(Frame &frame);

    RE flushAllPages();

    RE recoverPage(int32_t page_num);

protected:
    RE allocateFrame(int32_t page_num, Frame **buf);

    RE purgeFrame(int32_t page_num, Frame *used_frame);

    RE checkPageNum(int32_t page_num);

    RE loadPage(int32_t page_num, Frame *frame);

private:
    BufferPoolManager &bp_manager_;
    FrameManager &frame_manager_;
    std::string file_name_;
    int file_desc_ = -1;
    Frame *header_frame_ = nullptr;
    BPFileHeader *file_header_ = nullptr;
    std::set<int32_t> disposed_pages;

private:
    friend class BufferPoolIterator;
};

class BufferPoolManager {
public:
    static BufferPoolManager &getInstance();

    void initialize();

    RE createFile(const char *file_name);

    RE openFile(const char *file_name, DiskBufferPool *&bp);

    RE closeFile(const char *file_name);

    RE flushPage(Frame &frame);

    void destroy();

private:
    BufferPoolManager();

    FrameManager frame_manager_{"BufPool"};
    std::unordered_map<std::string, DiskBufferPool *> buffer_pools_;
    std::unordered_map<int, DiskBufferPool *> fd_buffer_pools_;
};

class BufferPoolIterator {
public:
    BufferPoolIterator() : current_page_num_(-1) {}

    ~BufferPoolIterator() {}

    RE initialize(DiskBufferPool &bp, int32_t start_page = 0);

    bool hasNext();

    int32_t next();

    RE reset();

private:
    BitMap bit_map_;
    int32_t current_page_num_ = -1;
};