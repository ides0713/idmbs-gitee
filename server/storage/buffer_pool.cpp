#include "buffer_pool.h"

Frame::Frame() : dirty_(false), pin_count_(0), acc_time_(0), file_desc_(-1)
{
}