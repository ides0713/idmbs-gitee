#pragma once
#include "../common/re.h"
#include "../common/server_defs.h"
#include "bitmap.h"
#include "lru_cache.h"
#include "mem_pool.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 14)// 2^14 16KB
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(int32_t))
#define BP_FILE_HDR_SIZE (sizeof(BPFileHeader))
///@brief unit of store data in the buffer
struct Page {
public:
    ///@brief divide the data of the file into pages,the page id begin from 0
    ///@NOTE pages are identified by page_id
    int32_t page_id;
    char page_data[BP_PAGE_DATA_SIZE];
};
///@brief first page of bp file
///@NOTE page of buffer pool file header 's id is 0;header page of bp file also has its corresponding on-disk structure
struct BufferPoolFileHeader {
public:
    ///@brief num of total pages of current file
    int32_t pages_num;
    ///@brief num of total allocated pages
    int32_t allocated_pages_num;
    ///@brief indicate which pages in the file have been put into the cache
    ///@LINK https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
    char bitmap[0];

public:
    ///@brief max num of allocatable pages,i.e. num of bit of bitmap;
    ///@NOTE file header is a page itself,therefore max page num is calculate this way
    static const int max_page_num = (BP_PAGE_DATA_SIZE - sizeof(pages_num) - sizeof(allocated_pages_num)) * 8;
};
///@brief frame is slot for page in the buffer
class Frame
{
public:
    ///@brief default construction function of the class frame
    Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1) {}
    ///@brief memset the corresponding page to 0
    void ResetPage() { memset(&page_, 0, sizeof(page_)); }
    ///@brief set dirty_ to true
    void DirtyMark() { dirty_ = true; };
    ///@brief getFrame page_id from corresponding page
    [[nodiscard]] int32_t GetPageId() const { return page_.page_id; };
    ///@brief set corresponding page's page_id with given param
    void SetPageId(int32_t id) { page_.page_id = id; }
    ///@brief getFrame page data from corresponding page
    char *GetPageData() { return page_.page_data; };
    ///@brief getFrame corresponding file desc
    [[nodiscard]] int GetFileDesc() const { return file_desc_; };
    ///@brief set corresponding file desc with given param
    void SetFileDesc(int file_desc) { file_desc_ = file_desc; }
    ///@brief return true if pin_count LE 0@n(todo:what is purgeable and pin_count stand for?)
    [[nodiscard]] bool IsPurgeable() const { return pin_count_ <= 0; }
    void Desc();

private:
    bool dirty_;// a frame is set dirty,i.e. it was 'changed' in memory and we need to write it to disk
    ///@brief pin count of the frame if pin count GT 0 the page is pinned
    ///@NOTE pin page means the make this page can not be replaced out of memory@n if the page is the frame of the header file page,the is pinned when pin count GT 1('difficult' to pin))
    unsigned int pin_count_;
    unsigned long acc_time_;// this member variable describes the time at when the frame was created
    int file_desc_;         // file_desc of the corresponding buffer pool
    Page page_;             // the corresponding page of the frame
private:
    friend class DiskBufferPool;
};
///@brief getFrame frame_id by hash file_desc and getPageId
class FrameId
{
public:
    FrameId(int file_desc, int32_t page_num) : file_desc_(file_desc), page_id_(page_num) {}
    ///@brief a class FrameId is equal to another one,i.e. the file desc and page_id both equal the other one's
    [[nodiscard]] bool EqualTo(const FrameId &other) const {
        return file_desc_ == other.file_desc_ && page_id_ == other.page_id_;
    }
    bool operator==(const FrameId &other) const { return this->EqualTo(other); }
    ///@brief get frame a hash number with the member variables of class FrameId
    [[nodiscard]] size_t Hash() const { return static_cast<size_t>(file_desc_) << 32L | page_id_; }
    ///@brief get frame corresponding buffer pool's file desc
    [[nodiscard]] int GetFileDesc() const { return file_desc_; }
    ///@brief get frame corresponding page's page_id
    [[nodiscard]] int32_t GetPageId() const { return page_id_; }

private:
    int file_desc_;  // file_desc of the corresponding buffer pool
    int32_t page_id_;// page_id of the corresponding page
};
///@brief manager of frames,providing handlers of class Frame to other classes
class FrameManager
{
public:
    explicit FrameManager(const char *tag) : frame_allocator_(tag) {}
    ///@brief openFile the memory pool of frames
    Re Init(int pool_num);
    ///@brief destruction
    Re CleanUp();
    ///@brief try to get pointer of frame with file_desc and page_id from lru
    ///@NOTE if can not find frame in lru,return nullptr
    Frame *GetFrame(int file_desc, int32_t page_id);
    ///@brief allocate a frame with file_desc and page_id in buffer pool,then return pointer of it
    ///@NOTE if find frame in lru,just return nullptr
    Frame *Alloc(int file_desc, int32_t page_id);
    ///@brief free the frame from lru and memory pool with provided params
    ///@NOTE if arg:frame not equal to frame found in lru cache then return error and do nothing
    Re Free(int file_desc, int32_t page_id, Frame *frame);
    ///@brief find purgeable frame from lru cache(if can not find from lru cache,return nullptr(no unpinned page that can be purged))
    Frame *GetPurgeableFrame();
    ///@brief get all frames for a file(buffer pool)
    std::list<Frame *> FindFrames(int file_desc);
    ///@brief get count of frames in Lru
    size_t GetFrameInLruCount() const { return frame_lru_cache_.Count(); }
    ///@brief get count of frames that allocator can provide to be allocator(includes frames that already be allocated)
    size_t GetTotalFrameCount() const { return frame_allocator_.GetSize(); }

private:
    class FrameIdHash
    {
    public:
        size_t operator()(const FrameId &frame_id) const { return frame_id.Hash(); }

    private:
    };

private:
    std::mutex lock_;
    LruCache<FrameId, Frame *, FrameIdHash, std::equal_to<FrameId>>
            frame_lru_cache_;// a lru cache of allocated frames@n(NOTE:the lru cache includes all allocated frames)
    MemoryPool<Frame> frame_allocator_;// a allocator of frames,implemented with memory pool
};
class GlobalBufferPoolManager;
///@brief a buffer pool of corresponding file,i.e. a buffer pool can be used by only one on-disk file
class DiskBufferPool
{
public:
    DiskBufferPool(GlobalBufferPoolManager &buffer_pool_manager, FrameManager &frame_manager)
        : buffer_pool_manager_(buffer_pool_manager), frame_manager_(frame_manager), file_desc_(-1),
          header_frame_(nullptr), file_header_(nullptr) {}
    ///@brief get target page with page_id,result passed by param:frame
    ///@NOTE if target page was already stored in a frame(in lru cache),get it with frame manager(lru cache),set param:frame to frame get from lru cache;if not,allocated a new frame and put page in it ,then set param:frame to the allocated frame)
    Re GetPage(int32_t page_id, Frame **frame);
    ///@brief allocate page in frame
    ///@NOTE set corresponding bit of bitmap to 1
    Re AllocatePage(Frame **frame);
    ///@brief if the page is purgeable,just purge it(purge page);if not,insert it into the set disposed_pages and dispose it later
    Re DisposePage(int32_t page_id);
    ///@brief unpin page means set pin_count-=1 and if the page was unpinned(pin count <=0 ) after unpinPage,check if this page is waiting to be disposed
    ///@NOTE there is an assert that pin_count of the frame GE 1
    Re UnpinPage(Frame *frame);
    ///@brief return allocated pages num
    Re GetAllocatedPagesNum(int *page_count);
    void CheckAllPages();
    ///@brief use for debug
    Re CheckAllPagesUnpinned();
    ///@brief return page
    [[nodiscard]] int GetFileDesc() const;
    ///@brief flush the page,i.e. if page is dirty,write data back to on-disk file
    ///@NOTE flush page does not dispose the page or move the page out of buffer
    Re FlushPage(Frame &frame);
    ///@brief check if corresponding bit of page_id in bitmap was 0,set it to 1
    Re RecoverPage(int32_t page_id);
    ///@brief destroy the file/unlink the buffer pool and the on-disk file
    Re CloseFile();
    void Destroy();

private:
    GlobalBufferPoolManager &buffer_pool_manager_;
    FrameManager &frame_manager_;
    std::string file_name_;
    int file_desc_;
    Frame *header_frame_;// header_frame's page id = 0(BP_HEADER_PAGE)
    BufferPoolFileHeader *file_header_;
    ///@brief pages in disposed_pages_ are those which should be disposed,but were pinned,so they cannot be disposed immediately
    ///@NOTE pages in disposed_pages_ will be tried to disposed again when the file is going to be closed or the page is unpinned(unpinPage)
    std::set<int32_t> disposed_pages_;

private:
    ///@brief if target page was stored in allocated frame,purge frame;if not,just do nothing and return success
    ///@NOTE purge page should only be used in CloseFile(purge header page) and implementation of dispose page,because purgePage did not apply the change in the bitmap
    Re PurgePage(int32_t page_id);
    ///@brief purge all page
    Re PurgeAllPages();
    ///@brief allocate a frame for page,result passed by param:buf(currently no data in frame.page,but frame id has already been set)
    Re AllocateFrame(int32_t page_id, Frame **buf);
    ///@brief flush page if dirty and then free the frame with frame manager
    Re PurgeFrame(int32_t page_id, Frame *used_frame);
    ///@brief check 1.if page_id is GT
    Re CheckPageId(int32_t page_id);
    ///@brief load a page from file to given frame
    Re LoadPage(int32_t page_id, Frame *frame);
    ///@brief flush all pages of the on-disk file in buffer
    Re FlushAllPages();
    // todo:ascertain the relationship between allocated_page and pages_num of file header
    ///@brief as a buffer pool correspond one on-disk file,openFile is initialization of the instance indeed
    Re OpenFile(std::string file_name);

private:
    friend class BufferPoolIterator;
    friend class GlobalBufferPoolManager;
};
class GlobalBufferPoolManager
{
public:
    GlobalBufferPoolManager() = default;
    ///@brief init the frame manager
    void Init();
    void Destroy();
    ///@brief open file,i.e. allocate a buffer for the target file and pass the buffer by param:bp
    Re OpenFile(std::string file_name, DiskBufferPool *&bp);
    ///@brief close file,i.e. destroy the buffer allocated to the file
    Re CloseFile(std::string file_name);
    ///@brief different from buffer.flushPage(flushPage will try to get frame )
    Re FlushPage(Frame &frame);

public:
    ///@brief createFilter a file with a special format(first page is header page with a bitmap and two params(pages_num and allocated_pages_num))
    ///@NOTE did not allocate a buffer for it,just createFilter it
    static Re CreateFile(const char *file_name);

private:
    FrameManager frame_manager_{"BufPool"};
    std::unordered_map<std::string, DiskBufferPool *> buffer_pools_;
    std::unordered_map<int, DiskBufferPool *> fd_buffer_pools_;
};
///@brief used to getFrame page in a buffer pool,is is implemented internally using bitmap
class BufferPoolIterator
{
public:
    BufferPoolIterator() : current_page_id_(-1) {}
    Re Init(DiskBufferPool &bp, int32_t start_page = 0);
    bool HasNext();
    int32_t Next();
    Re Reset();
    ///@brief debug use
    int32_t GetCurrentPageId() { return current_page_id_; }

private:
    BitMap bit_map_;
    int32_t current_page_id_;
};