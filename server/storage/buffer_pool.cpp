#include "buffer_pool.h"
#include <string.h>
Page::Page()
{
}
Frame::Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1)
{
}

void Frame::resetPage()
{
    memset(&page_, 0, sizeof(page_));
}

void Frame::dirtyMark()
{
    dirty_ = true;
}

int32_t Frame::getPageId()
{
    return page_.page_id_;
}

void Frame::setPageId(int32_t id)
{
    page_.page_id_ = id;
}

char *Frame::getPageData()
{
    return page_.page_data_;
}

int Frame::getFileDesc()
{
    return file_desc_;
}

void Frame::setFileDesc(int file_desc)
{
    file_desc_ = file_desc;
}

bool Frame::isPurgable()
{
    return pin_count_ <= 0;
}
