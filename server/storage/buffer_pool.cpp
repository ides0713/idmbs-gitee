#include "buffer_pool.h"
RE FrameManager::initialize(int pool_num)
{
  int ret = allocator_.initialize(false, pool_num);
  if (ret == 0)
  {
    return RE::SUCCESS;
  }
  return RE::FAIL;
}

RE FrameManager::cleanUp()
{
  if (frames_.count() > 0)
    return RE::FAIL;
  frames_.destroy();
  return RE::SUCCESS;
}

Frame *FrameManager::beginPurge()
{
  Frame *purgable_frame = nullptr;
  auto purge_finder = [&purgable_frame](const FrameId &frame_id, Frame *const frame)
  {
    if (frame->isPurgable())
    {
      purgable_frame = frame;
      return false; // false to break the progress
    }
    return true; // true continue to look up
  };
  frames_.foreachReverse(purge_finder);
  return purgable_frame;
}

Frame *FrameManager::get(int file_desc, int32_t page_num)
{
  FrameId frame_id(file_desc, page_num);
  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame = nullptr;
  ///@brief 显明丢弃该值
  (void)frames_.get(frame_id, frame);
  return frame;
}

Frame *FrameManager::alloc(int file_desc, int32_t page_num)
{
  FrameId frame_id(file_desc, page_num);
  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame = nullptr;
  bool found = frames_.get(frame_id, frame);
  if (found)
    return nullptr;
  frame = allocator_.alloc();
  if (frame != nullptr)
    frames_.put(frame_id, frame);
  return frame;
}

RE FrameManager::free(int file_desc, int32_t page_num, Frame *frame)
{
  FrameId frame_id(file_desc, page_num);

  std::lock_guard<std::mutex> lock_guard(lock_);
  Frame *frame_source = nullptr;
  bool found = frames_.get(frame_id, frame_source);
  if (!found or frame != frame_source)
  {
    printf("failed to find frame or got frame not match. file_desc=%d, PageNum=%d, frame_source=%p, frame=%p",
           file_desc, page_num, frame_source, frame);
    return RE::FAIL;
  }
  frames_.remove(frame_id);
  allocator_.free(frame);
  return RE::SUCCESS;
}
std::list<Frame *> FrameManager::find_list(int file_desc)
{
  std::lock_guard<std::mutex> lock_guard(lock_);
  std::list<Frame *> frames;
  auto fetcher = [&frames, file_desc](const FrameId &frame_id, Frame * const frame) -> bool {
    if (file_desc == frame_id.getFileDesc())
          frames.push_back(frame);
    return true;
  };
  frames_.foreach(fetcher);
  return frames;
}