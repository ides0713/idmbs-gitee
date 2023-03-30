#pragma once

#include <cstdint>
#include "LRU_cache.h"
#include "mem_pool.h"
#include "bitmap.h"
#include <cstring>
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

    friend class GlobalBufferPoolManager;
};

// first page of bpfile
struct BufferPoolFileHeader {
    int32_t page_count;      //! 当前文件一共有多少个页面
    int32_t allocated_pages; //! 已经分配了多少个页面
    // 指向数据后一位 (新鲜的用法)
    char bitmap[0]; //! 页面分配位图, 第0个页面(就是当前页面)，总是1

    /**
     * 能够分配的最大的页面个数，即bitmap的字节数 乘以8
     */
    // file header是一页 使用该页存储 字节*8个 bit
    static const int max_page_num = (BP_PAGE_DATA_SIZE - sizeof(page_count) - sizeof(allocated_pages)) * 8;
};

class Frame {
public:
    Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1) {}

    void resetPage() { memset(&page_, 0, sizeof(page_)); }

    void dirtyMark() { dirty_ = true; };

    [[nodiscard]] int32_t getPageId() const { return page_.page_id_; };

    void setPageId(int32_t id) { page_.page_id_ = id; }

    char *getPageData() { return page_.page_data_; };

    // void setPageData(const char * data);
    [[nodiscard]] int getFileDesc() const { return file_desc_; };

    void setFileDesc(int file_desc) { file_desc_ = file_desc; }

    [[nodiscard]] bool isPurgable() const { return pin_count_ <= 0; }

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

    [[nodiscard]] bool equalTo(const FrameId &other) const {
        return file_desc_ == other.file_desc_ && page_id_ == other.page_id_;
    }

    bool operator==(const FrameId &other) const { return this->equalTo(other); }

    [[nodiscard]] size_t hash() const { return static_cast<size_t>(file_desc_) << 32L | page_id_; }

    [[nodiscard]] int getFileDesc() const { return file_desc_; }

    [[nodiscard]] int32_t getPageId() const { return page_id_; }

private:
    int file_desc_;
    int32_t page_id_;
};

class FrameManager {
public:
    explicit FrameManager(const char *tag) : memory_pool_allocator_(tag) {}

    Re initialize(int pool_num);

    Re cleanUp();

    Frame *get(int file_desc, int32_t page_num);

    Frame *alloc(int file_desc, int32_t page_num);

    Re free(int file_desc, int32_t page_num, Frame *frame);

    Frame *beginPurge();

    std::list<Frame *> findList(int file_desc);

    size_t getFrameNum() const { return frames_lru_cache_.count(); }

    size_t getTotalFrameNum() const { return memory_pool_allocator_.getSize(); }

private:
    class FrameIdHasher {
    public:
        size_t operator()(const FrameId &frame_id) const { return frame_id.hash(); }

    private:
    };

private:
    std::mutex lock_;
    LruCache<FrameId, Frame *, FrameIdHasher, std::equal_to<FrameId>> frames_lru_cache_;
    MemoryPool<Frame> memory_pool_allocator_;
};

class GlobalBufferPoolManager;
///@brief class above does not make on-disk change

class DiskBufferPool {
public:
    DiskBufferPool(GlobalBufferPoolManager &bp_manager, FrameManager &frame_manager) : bp_manager_(bp_manager),
                                                                                       frame_manager_(frame_manager) {}

    ~DiskBufferPool();

    // Re create_file(const char *file_name);
    Re openFile(const char *file_name);

    Re closeFile();

    Re getThisPage(int32_t page_num, Frame **frame);

    Re allocatePage(Frame **frame);

    Re disposePage(int32_t page_num);

    Re purgePage(int32_t page_num);

    Re purgeAllPages();

    Re unpinPage(Frame *frame);

    Re getPageCount(int *page_count);

    Re checkAllPagesUnpinned();

    [[nodiscard]] int getFileDesc() const;

    Re flushPage(Frame &frame);

    Re flushAllPages();

    Re recoverPage(int32_t page_num);

protected:
    Re allocateFrame(int32_t page_num, Frame **buf);

    Re purgeFrame(int32_t page_num, Frame *used_frame);

    Re checkPageNum(int32_t page_num);

    Re loadPage(int32_t page_num, Frame *frame);

private:
    GlobalBufferPoolManager &bp_manager_;
    FrameManager &frame_manager_;
    std::string file_name_;
    int file_desc_ = -1;
    Frame *header_frame_ = nullptr;
    BufferPoolFileHeader *file_header_ = nullptr;
    std::set<int32_t> disposed_pages_;

private:
    friend class BufferPoolIterator;
};

class GlobalBufferPoolManager {
public:
    GlobalBufferPoolManager() = default;

    void initialize();

    Re createFile(const char *file_name);

    Re openFile(const char *file_name, DiskBufferPool *&bp);

    Re closeFile(const char *file_name);

    Re flushPage(Frame &frame);

    void destroy();

private:

    FrameManager frame_manager_{"BufPool"};
    std::unordered_map<std::string, DiskBufferPool *> buffer_pools_;
    std::unordered_map<int, DiskBufferPool *> fd_buffer_pools_;
};

class BufferPoolIterator {
public:
    BufferPoolIterator() : current_page_num_(-1) {}

    ~BufferPoolIterator() {}

    Re initialize(DiskBufferPool &bp, int32_t start_page = 0);

    bool hasNext();

    int32_t next();

    Re reset();

private:
    BitMap bit_map_;
    int32_t current_page_num_ = -1;
};