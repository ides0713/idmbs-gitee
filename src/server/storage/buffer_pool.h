#pragma once

#include <cstdint>
#include <cstdio>
#include "lru_cache.h"
#include "mem_pool.h"
#include "bitmap.h"
#include <cstring>
#include <unordered_map>
#include "../common/server_defs.h"
#include "../common/re.h"

#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 14) //2^14
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(int32_t))
#define BP_FILE_HDR_SIZE (sizeof(BPFileHeader))

///@brief unit of store data in the buffer
class Page {
public:
    int32_t page_id;
    char page_data[BP_PAGE_DATA_SIZE];
};

///@brief first page of bp file
struct BufferPoolFileHeader {
public:
    ///@brief num of total pages of current file
    int32_t page_count;
    ///@brief num of total allocated pages
    int32_t allocated_pages;
    ///@brief array with size 0,means a variable-length array@n(LINK:https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html)
    char bitmap[0];
public:
    ///@brief max num of allocatable pages,i.e. num of bit of bitmap;@n(note:file header is a page itself,therefore max page num is calculate this way)
    static const int max_page_num = (BP_PAGE_DATA_SIZE - sizeof(page_count) - sizeof(allocated_pages)) * 8;
};

///@brief frame is slot for page in the buffer
class Frame {
public:
    ///@brief default construction function of the class frame
    Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1) {}

    ///@brief memset the corresponding page to 0
    void resetPage() { memset(&page_, 0, sizeof(page_)); }

    ///@brief set dirty_ to true
    void dirtyMark() { dirty_ = true; };

    ///@brief getFrame page_id from corresponding page
    [[nodiscard]] int32_t getPageId() const { return page_.page_id; };

    ///@brief set corresponding page's page_id with given param
    void setPageId(int32_t id) { page_.page_id = id; }

    ///@brief getFrame page data from corresponding page
    char *getPageData() { return page_.page_data; };

    ///@brief getFrame corresponding file desc
    [[nodiscard]] int getFileDesc() const { return file_desc_; };

    ///@brief set corresponding file desc with given param
    void setFileDesc(int file_desc) { file_desc_ = file_desc; }

    ///@brief return true if pin_count LE 0@n(todo:what is purgeable and pin_count stand for?)
    [[nodiscard]] bool isPurgeable() const { return pin_count_ <= 0; }

private:
    bool dirty_;//a frame is set dirty,i.e. it was 'changed' in memory and we need to write it to disk
    ///@brief pin count of the frame if pin count GT 0 the page is pinned
    ///@n(NOTE:pin page means the make this page can not be replaced out of memory)
    ///@n(NOTE:if the page is the frame of the header file page,the is pinned when pin count GT 1('difficult' to pin))
    unsigned int pin_count_;
    unsigned long acc_time_;//this member variable describes the time at when the frame was created
    int file_desc_;//file_desc of the corresponding buffer pool
    Page page_;//the corresponding page of the frame

private:
    friend class DiskBufferPool;
};

///@brief getFrame frame_id by hash file_desc and getPageId
class FrameId {
public:
    FrameId(int file_desc, int32_t page_num) : file_desc_(file_desc), page_id_(page_num) {}

    ///@brief a class FrameId is equal to another one,i.e. the file desc and page_id both equal the other one's
    [[nodiscard]] bool equalTo(const FrameId &other) const {
        return file_desc_ == other.file_desc_ && page_id_ == other.page_id_;
    }

    bool operator==(const FrameId &other) const { return this->equalTo(other); }

    ///@brief get frame a hash number with the member variables of class FrameId
    [[nodiscard]] size_t hash() const { return static_cast<size_t>(file_desc_) << 32L | page_id_; }

    ///@brief get frame corresponding buffer pool's file desc
    [[nodiscard]] int getFileDesc() const { return file_desc_; }

    ///@brief get frame corresponding page's page_id
    [[nodiscard]] int32_t getPageId() const { return page_id_; }

private:
    int file_desc_;//file_desc of the corresponding buffer pool
    int32_t page_id_;//page_id of the corresponding page
};

///@brief manager of frames,providing handlers of class Frame to other classes
class FrameManager {
public:
    explicit FrameManager(const char *tag) : frame_allocator_(tag) {}

    ///@brief initialize the memory pool of frames
    Re initialize(int pool_num);

    ///@brief destruction
    Re cleanUp();

    ///@brief try to get pointer of frame with file_desc and page_id from lru@n(NOTE:if can not find frame in lru,return nullptr)
    Frame *getFrame(int file_desc, int32_t page_id);

    ///@brief allocate a frame with file_desc and page_id in buffer pool,then return pointer of it@n(NOTE:if find frame in lru,just return nullptr)
    Frame *alloc(int file_desc, int32_t page_id);

    ///@brief free the frame from lru and memory pool with provided params@n(NOTE:if arg:frame not equal to frame found in lru cache then return error and do nothing)
    Re free(int file_desc, int32_t page_id, Frame *frame);

    ///@brief find purgeable frame from lru cache(if can not find from lru cache,return nullptr(no unpinned page that can be purged))
    Frame *getPurgeableFrame();

    ///@brief get all frames for a file(buffer pool)
    std::list<Frame *> findFrames(int file_desc);

    ///@brief get count of frames in Lru
    size_t getFrameInLruCount() const { return frame_lru_cache_.count(); }

    ///@brief get count of frames that allocator can provide to be allocator(includes frames that already be allocated)
    size_t getTotalFrameCount() const { return frame_allocator_.getSize(); }

private:
    class FrameIdHash {
    public:
        size_t operator()(const FrameId &frame_id) const { return frame_id.hash(); }

    private:
    };

private:
    std::mutex lock_;
    LruCache<FrameId, Frame *, FrameIdHash, std::equal_to<FrameId>> frame_lru_cache_;//a lru cache of allocated frames@n(NOTE:the lru cache includes all allocated frames)
    MemoryPool<Frame> frame_allocator_;//a allocator of frames,implemented with memory pool
};

class GlobalBufferPoolManager;

///@brief a buffer pool of corresponding file,i.e. a buffer pool can be used by only one on-disk file
class DiskBufferPool {
public:

    DiskBufferPool(GlobalBufferPoolManager &bp_manager, FrameManager &frame_manager) :
            bp_manager_(bp_manager), frame_manager_(frame_manager),
            file_desc_(-1), header_frame_(nullptr), file_header_(nullptr) {}

    ~DiskBufferPool();

    //todo:ascertain the relationship between allocated_page and page_count of file header
    ///@brief as a buffer pool correspond one on-disk file,openFile is initialization of the instance indeed
    Re openFile(const char *file_name);

    ///@brief close the file means unlink the buffer pool and the on-disk file
    Re closeFile();

    ///@brief get target page with page_id,result passed by param:frame@n(NOTE:if target page was already stored in a frame(in lru cache),get it with frame manager(lru cache),set param:frame to frame get from lru cache;if not,allocated a new frame and put page in it ,then set param:frame to the allocated frame)
    Re getPage(int32_t page_id, Frame **frame);

    ///@brief allocate page in frame@n(NOTE:if there is usable frame,just put page in it;if not,allocate a frame with frame manager,and then put page in it)
    Re allocatePage(Frame **frame);

    Re disposePage(int32_t page_id);

    Re purgePage(int32_t page_num);

    Re purgeAllPages();

    Re unpinPage(Frame *frame);

    Re getPageCount(int *page_count);

    Re checkAllPagesUnpinned();

    [[nodiscard]] int getFileDesc() const;

    Re flushPage(Frame &frame);

    Re flushAllPages();

    Re recoverPage(int32_t page_id);

protected:
    ///@brief allocate a frame for page,result passed by param:buf(currently no data in frame.page,but frame id has already been set)
    Re allocateFrame(int32_t page_id, Frame **buf);

    ///@brief try to free the frame
    Re purgeFrame(int32_t page_id, Frame *used_frame);

    Re checkPageId(int32_t page_id);

    ///@brief load a page from file to given frame
    Re loadPage(int32_t page_id, Frame *frame);

private:
    GlobalBufferPoolManager &bp_manager_;
    FrameManager &frame_manager_;
    std::string file_name_;
    int file_desc_;
    Frame *header_frame_;//header_frame's page id = 0(BP_HEADER_PAGE)
    BufferPoolFileHeader *file_header_;
    std::set<int32_t> disposed_pages_;

private:
    friend class BufferPoolIterator;
};

class GlobalBufferPoolManager {
public:
    GlobalBufferPoolManager() = default;

    void initialize();

    Re openFile(const char *file_name, DiskBufferPool *&bp);

    Re closeFile(const char *file_name);

    Re flushPage(Frame &frame);

    void destroy();

public:
    static Re createFile(const char *file_name);

private:
    FrameManager frame_manager_{"BufPool"};
    std::unordered_map<std::string, DiskBufferPool *> buffer_pools_;
    std::unordered_map<int, DiskBufferPool *> fd_buffer_pools_;
};

///@brief used to getFrame page in a buffer pool,is is implemented internally using bitmap
class BufferPoolIterator {
public:
    BufferPoolIterator() : current_page_num_(-1) {}

    Re init(DiskBufferPool &bp, int32_t start_page = 0);

    bool hasNext();

    int32_t next();

    Re reset();

private:
    BitMap bit_map_;
    int32_t current_page_num_ = -1;
};