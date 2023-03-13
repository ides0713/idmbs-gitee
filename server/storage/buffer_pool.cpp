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

void Frame::setPageId(int32_t id)
{
    page_.page_id_ = id;
}

void Frame::setFileDesc(int file_desc)
{
    file_desc_ = file_desc;
}