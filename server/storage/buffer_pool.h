#pragma once
#include <stdint.h>
#include "LRU_cache.h"
#include "mem_pool.h"
#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 14)
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(int32_t))
#define BP_FILE_HDR_SIZE (sizeof(BPFileHeader))
class Page
{
public:
    Page();

private:
    int32_t page_id_;
    char page_data_[BP_PAGE_SIZE];
    friend class Frame;
};
// first page of bpfile
struct BPFileHeader
{
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

class Frame
{
public:
    Frame();
    void resetPage();
    void dirtyMark() { dirty_ = true; };
    int32_t getPageId() const { return page_.page_id_; };
    void setPageId(int32_t id);
    char *getPageData() { return page_.page_data_; };
    // void setPageData(const char * data);
    int getFileDesc() const { return file_desc_; };
    void setFileDesc(int file_desc);
    bool isPurgable() { return pin_count_ <= 0; }

private:
    bool dirty_ = false;
    unsigned int pin_count_ = 0;
    unsigned long acc_time_ = 0;
    int file_desc_ = -1;
    Page page_;
};

class FrameId
{
public:
    FrameId(int file_desc, int32_t page_num) : file_desc_(file_desc), page_id_(page_num) {}

    bool equal_to(const FrameId &other) const
    {
        return file_desc_ == other.file_desc_ && page_id_ == other.page_id_;
    }

    bool operator==(const FrameId &other) const
    {
        return this->equal_to(other);
    }

    size_t hash() const
    {
        return static_cast<size_t>(file_desc_) << 32L | page_id_;
    }

    int getFileDesc() const { return file_desc_; }
    int32_t getPageId() const { return page_id_; }

private:
    int file_desc_;
    int32_t page_id_;
};

class FrameManager{

};