#include "buffer_pool.h"
RE FrameManager::initialize(int pool_num)
{
  bool ret = memory_pool_allocator_.initialize(false, pool_num);
  if (ret)
    return RE::SUCCESS;
  return RE::FAIL;
}

RE FrameManager::cleanUp()
{
  if (frames_LRU_cache_.count() > 0)
    return RE::FAIL;
  frames_LRU_cache_.destroy();
  return RE::SUCCESS;
}

Frame *FrameManager::get(int file_desc, int32_t page_num)
{
  FrameId frame_id(file_desc, page_num);
  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame = nullptr;
  ///@brief 显明丢弃该值
  (void)frames_LRU_cache_.get(frame_id, frame);
  return frame;
}

Frame *FrameManager::alloc(int file_desc, int32_t page_num)
{
  FrameId frame_id(file_desc, page_num);
  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame = nullptr;
  bool found = frames_LRU_cache_.get(frame_id, frame);
  if (found)
    return nullptr;
  frame = memory_pool_allocator_.alloc();
  if (frame != nullptr)
    frames_LRU_cache_.put(frame_id, frame);
  return frame;
}

RE FrameManager::free(int file_desc, int32_t page_num, Frame *frame)
{
  FrameId frame_id(file_desc, page_num);
  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame_source = nullptr;
  bool found = frames_LRU_cache_.get(frame_id, frame_source);
  if (!found or frame != frame_source)
  {
    printf("failed to find frame or got frame not match. file_desc=%d, PageNum=%d, frame_source=%p, frame=%p",
           file_desc, page_num, frame_source, frame);
    return RE::FAIL;
  }
  frames_LRU_cache_.remove(frame_id);
  memory_pool_allocator_.free(frame);
  return RE::SUCCESS;
}

Frame *FrameManager::beginPurge()
{
  Frame *purgable_frame = nullptr;
  auto purge_finder = [&purgable_frame](const FrameId &frame_id, Frame *const frame) -> bool
  {
    if (frame->isPurgable())
    {
      purgable_frame = frame;
      return false; // false to break the progress
    }
    return true; // true continue to look up
  };
  ///@brief LRU淘汰页面 (从cache中)
  frames_LRU_cache_.foreachReverse(purge_finder);
  return purgable_frame;
}

std::list<Frame *> FrameManager::find_list(int file_desc)
{
  std::lock_guard<std::mutex> lock_guard(lock_);
  std::list<Frame *> frames;
  auto fetcher = [&frames, file_desc](const FrameId &frame_id, Frame *const frame) -> bool
  {
    if (file_desc == frame_id.getFileDesc())
      frames.push_back(frame);
    return true;
  };
  frames_LRU_cache_.foreach (fetcher);
  return frames;
}

RE BufferPoolIterator::initialize(DiskBufferPool &bp, int32_t start_page /* = 0 */)
{
  bit_map_.initialize(bp.file_header_->bitmap, bp.file_header_->page_count);
  if (start_page <= 0)
    current_page_num_ = 0;
  else
    current_page_num_ = start_page;
  return RE::SUCCESS;
}

bool BufferPoolIterator::hasNext()
{
  return bit_map_.nextSettedBit(current_page_num_ + 1) != -1;
}

int32_t BufferPoolIterator::next()
{
  int32_t next_page = bit_map_.nextSettedBit(current_page_num_ + 1);
  if (next_page != -1)
    current_page_num_ = next_page;
  return next_page;
}

RE BufferPoolIterator::reset()
{
  current_page_num_ = 0;
  return RE::SUCCESS;
}