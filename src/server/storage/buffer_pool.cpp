#include "buffer_pool.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static const int32_t BP_HEADER_PAGE = 0;
static const int MEM_POOL_ITEM_NUM = 128;

unsigned long getCurrentTime()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec * 1000 * 1000 * 1000UL + tp.tv_nsec;
}

int writen(int fd, const void *buf, int size)
{
  const char *tmp = (const char *)buf;
  while (size > 0)
  {
    const __ssize_t ret = write(fd, tmp, size);
    if (ret >= 0)
    {
      tmp += ret;
      size -= ret;
      continue;
    }
    const int err = errno;
    if (EAGAIN != err && EINTR != err)
      return err;
  }
  return 0;
}

int readn(int fd, void *buf, int size)
{
  char *tmp = (char *)buf;
  while (size > 0)
  {
    const ssize_t ret = ::read(fd, tmp, size);
    if (ret > 0)
    {
      tmp += ret;
      size -= ret;
      continue;
    }
    if (0 == ret)
      return -1; // end of file

    const int err = errno;
    if (EAGAIN != err && EINTR != err)
      return err;
  }
  return 0;
}

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

std::list<Frame *> FrameManager::findList(int file_desc)
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

DiskBufferPool::~DiskBufferPool()
{
  closeFile();
  printf("DiskBufferPool:Exit\n");
}

RE DiskBufferPool::openFile(const char *file_name)
{
  int fd;
  if ((fd = open(file_name, O_RDWR)) < 0)
  {
    printf("Failed to open file %s, because %s.\n", file_name, strerror(errno));
    return RE::FAIL;
  }
  printf("Successfully open file %s.\n", file_name);
  file_name_ = file_name;
  file_desc_ = fd;
  RE re = RE::SUCCESS;
  re = allocateFrame(BP_HEADER_PAGE, &header_frame_);
  if (re != RE::SUCCESS)
  {
    printf("failed to allocate frame for header. file name %s\n", file_name_.c_str());
    close(fd);
    file_desc_ = -1;
    return re;
  }
  header_frame_->dirty_ = false;
  header_frame_->file_desc_ = fd;
  header_frame_->pin_count_ = 1;
  header_frame_->acc_time_ = getCurrentTime();
  if ((re = loadPage(BP_HEADER_PAGE, header_frame_)) != RE::SUCCESS)
  {
    printf("Failed to load first page of %s, due to %s.\n", file_name, strerror(errno));
    header_frame_->pin_count_ = 0;
    purgeFrame(BP_HEADER_PAGE, header_frame_);
    close(fd);
    file_desc_ = -1;
    return re;
  }
  file_header_ = (BPFileHeader *)header_frame_->getPageData();
  printf("Successfully open %s. file_desc=%d, hdr_frame=%p\n", file_name, file_desc_, header_frame_);
  return RE::SUCCESS;
}

RE DiskBufferPool::closeFile()
{
  RE re = RE::SUCCESS;
  if (file_desc_ < 0)
  {
    return re;
  }

  header_frame_->pin_count_--;
  // TODO: 理论上是在回放时回滚未提交事务，但目前没有undo log，因此不下刷数据page，只通过redo log回放
  if ((re = purgePage(0)) != RE::SUCCESS)
  {
    header_frame_->pin_count_++;
    printf("Failed to close %s, due to failed to purge all pages.\n", file_name_.c_str());
    return re;
  }

  disposed_pages.clear();

  if (close(file_desc_) < 0)
  {
    printf("Failed to close fileId:%d, fileName:%s, error:%s\n", file_desc_, file_name_.c_str(), strerror(errno));
    return RE::FAIL;
  }
  printf("Successfully close file %d:%s.\n", file_desc_, file_name_.c_str());
  file_desc_ = -1;

  bp_manager_.closeFile(file_name_.c_str());
  return RE::SUCCESS;
}

RE DiskBufferPool::getThisPage(int32_t page_num, Frame **frame)
{
  RE re = RE::SUCCESS;
  Frame *used_match_frame = frame_manager_.get(file_desc_, page_num);
  if (used_match_frame != nullptr)
  {
    used_match_frame->pin_count_++;
    used_match_frame->acc_time_ = getCurrentTime();
    *frame = used_match_frame;
    return RE::SUCCESS;
  }
  // Allocate one page and load the data into this page
  Frame *allocated_frame = nullptr;
  if ((re = allocateFrame(page_num, &allocated_frame)) != RE::SUCCESS)
  {
    printf("Failed to alloc frame %s:%d, due to failed to alloc page.\n", file_name_.c_str(), page_num);
    return re;
  }
  allocated_frame->dirty_ = false;
  allocated_frame->file_desc_ = file_desc_;
  allocated_frame->pin_count_ = 1;
  allocated_frame->acc_time_ = getCurrentTime();
  if ((re = loadPage(page_num, allocated_frame)) != RE::SUCCESS)
  {
    printf("Failed to load page %s:%d\n", file_name_.c_str(), page_num);
    allocated_frame->pin_count_ = 0;
    purgeFrame(page_num, allocated_frame);
    return re;
  }
  *frame = allocated_frame;
  return RE::SUCCESS;
}

RE DiskBufferPool::allocatePage(Frame **frame)
{
  RE re = RE::SUCCESS;
  int byte = 0, bit = 0;
  if ((file_header_->allocated_pages) < (file_header_->page_count))
  {
    for (int i = 0; i < file_header_->page_count; i++)
    {
      byte = i / 8;
      bit = i % 8;
      if (((file_header_->bitmap[byte]) & (1 << bit)) == 0)
      {
        (file_header_->allocated_pages)++;
        file_header_->bitmap[byte] |= (1 << bit);
        // TODO,  do we need clean the loaded page's data?
        header_frame_->dirtyMark();
        return getThisPage(i, frame);
      }
    }
  }

  if (file_header_->page_count >= BPFileHeader::MAX_PAGE_NUM)
  {
    printf("file buffer pool is full. page count %d, max page count %d\n",
           file_header_->page_count, BPFileHeader::MAX_PAGE_NUM);
    return RE::FAIL;
  }
  int32_t page_num = file_header_->page_count;
  Frame *allocated_frame = nullptr;
  if ((re = allocateFrame(page_num, &allocated_frame)) != RE::SUCCESS)
  {
    printf("Failed to allocate frame %s, due to no free page.\n", file_name_.c_str());
    return re;
  }

  file_header_->allocated_pages++;
  file_header_->page_count++;

  byte = page_num / 8;
  bit = page_num % 8;
  file_header_->bitmap[byte] |= (1 << bit);
  header_frame_->dirtyMark();

  allocated_frame->dirty_ = false;
  allocated_frame->file_desc_ = file_desc_;
  allocated_frame->pin_count_ = 1;
  allocated_frame->acc_time_ = getCurrentTime();
  allocated_frame->resetPage();
  allocated_frame->page_.page_id_ = file_header_->page_count - 1;

  // Use flush operation to extension file
  if ((re = flushPage(*allocated_frame)) != RE::SUCCESS)
  {
    printf("Failed to alloc page %s , due to failed to extend one page.\n", file_name_.c_str());
    // skip return false, delay flush the extended page
    // return tmp;
  }
  *frame = allocated_frame;
  return RE::SUCCESS;
}

RE DiskBufferPool::disposePage(int32_t page_num)
{
  RE re = purgePage(page_num);
  if (re != RE::SUCCESS)
  {
    printf("Dispose page %s:%d later, due to this page is being used\n", file_name_.c_str(), page_num);
    disposed_pages.insert(page_num);
    return re;
  }
  header_frame_->dirty_ = true;
  file_header_->allocated_pages--;
  char tmp = 1 << (page_num % 8);
  file_header_->bitmap[page_num / 8] &= ~tmp;
  return RE::SUCCESS;
}

RE DiskBufferPool::purgePage(int32_t page_num)
{
  Frame *used_frame = frame_manager_.get(file_desc_, page_num);
  if (used_frame != nullptr)
    return purgeFrame(page_num, used_frame);
  return RE::SUCCESS;
}

RE DiskBufferPool::purgeAllPages()
{
  std::list<Frame *> used = frame_manager_.findList(file_desc_);
  for (std::list<Frame *>::iterator it = used.begin(); it != used.end(); ++it)
  {
    Frame *frame = *it;
    if (frame->pin_count_ > 0)
    {
      printf("The page has been pinned, file_desc:%d, pagenum:%d, pin_count=%d\n",
             frame->file_desc_, frame->page_.page_id_, frame->pin_count_);
      continue;
    }
    if (frame->dirty_)
    {
      RE re = flushPage(*frame);
      if (re != RE::SUCCESS)
      {
        printf("Failed to flush all pages' of %s.\n", file_name_.c_str());
        return re;
      }
    }
    frame_manager_.free(file_desc_, frame->page_.page_id_, frame);
  }
  return RE::SUCCESS;
}

RE DiskBufferPool::unpinPage(Frame *frame)
{
  assert(frame->pin_count_ >= 1);
  if (--frame->pin_count_ == 0)
  {
    int32_t page_num = frame->getPageId();
    auto pages_it = disposed_pages.find(page_num);
    if (pages_it != disposed_pages.end())
    {
      printf("Dispose file_desc:%d, page:%d\n", file_desc_, page_num);
      disposePage(page_num);
      disposed_pages.erase(pages_it);
    }
  }
  return RE::SUCCESS;
}

RE DiskBufferPool::getPageCount(int *page_count)
{
  *page_count = file_header_->allocated_pages;
  return RE::SUCCESS;
}

RE DiskBufferPool::checkAllPagesUnpinned()
{
  std::list<Frame *> frames = frame_manager_.findList(file_desc_);
  for (auto &frame : frames)
  {
    if (frame->getPageId() == BP_HEADER_PAGE && frame->pin_count_ > 1)
      printf("This page has been pinned. file desc=%d, page num:%d, pin count=%d\n",
             file_desc_, frame->getPageId(), frame->pin_count_);
    else if (frame->getPageId() != BP_HEADER_PAGE && frame->pin_count_ > 0)
      printf("This page has been pinned. file desc=%d, page num:%d, pin count=%d\n",
             file_desc_, frame->getPageId(), frame->pin_count_);
  }
  printf("all pages have been checked of file desc %d\n", file_desc_);
  return RE::SUCCESS;
}

int DiskBufferPool::getFileDesc() const
{
  return file_desc_;
}

RE DiskBufferPool::flushPage(Frame &frame)
{
  Page &page = frame.page_;
  long long offset = ((long long)page.page_id_) * sizeof(Page);
  if (lseek(file_desc_, offset, SEEK_SET) == offset - 1)
  {
    printf("Failed to flush page %lld of %d due to failed to seek %s.\n", offset, file_desc_, strerror(errno));
    return RE::FAIL;
  }
  if (writen(file_desc_, &page, sizeof(Page)) != 0)
  {
    printf("Failed to flush page %lld of %d due to %s.\n", offset, file_desc_, strerror(errno));
    return RE::FAIL;
  }
  frame.dirty_ = false;
  printf("Flush block. file desc=%d, page num=%d", file_desc_, page.page_id_);

  return RE::SUCCESS;
}

RE DiskBufferPool::flushAllPages()
{
  std::list<Frame *> used = frame_manager_.findList(file_desc_);
  for (Frame *frame : used)
  {
    RE re = flushPage(*frame);
    if (re != RE::SUCCESS)
    {
      printf("failed to flush all pages\n");
      return re;
    }
  }
  return RE::SUCCESS;
}

RE DiskBufferPool::recoverPage(int32_t page_num)
{
  int byte = 0, bit = 0;
  byte = page_num / 8;
  bit = page_num % 8;
  if (!(file_header_->bitmap[byte] & (1 << bit)))
  {
    file_header_->bitmap[byte] |= (1 << bit);
    file_header_->allocated_pages++;
    file_header_->page_count++;
    header_frame_->dirtyMark();
  }
  return RE::SUCCESS;
}

RE DiskBufferPool::allocateFrame(int32_t page_num, Frame **buf)
{
  while (true)
  {
    Frame *frame = frame_manager_.alloc(file_desc_, page_num);
    if (frame != nullptr)
    {
      *buf = frame;
      return RE::SUCCESS;
    }
    frame = frame_manager_.beginPurge();
    if (frame == nullptr)
    {
      printf("All pages have been used and pinned.\n");
      return RE::FAIL;
    }
    if (frame->dirty_)
    {
      RE re = bp_manager_.flushPage(*frame);
      if (re != RE::SUCCESS)
      {
        printf("Failed to aclloc block due to failed to flush old block.\n");
        return re;
      }
    }
    frame_manager_.free(frame->getFileDesc(), frame->getPageId(), frame);
  }
  return RE::FAIL;
}

RE DiskBufferPool::purgeFrame(int32_t page_num, Frame *used_frame)
{
  if (used_frame->pin_count_ > 0)
  {
    printf("Begin to free page %d of %d(file id), but it's pinned, pin_count:%d.\n",
           used_frame->getPageId(), used_frame->file_desc_, used_frame->pin_count_);
    return RE::FAIL;
  }
  if (used_frame->dirty_)
  {
    RE rc = flushPage(*used_frame);
    if (rc != RE::SUCCESS)
    {
      printf("Failed to flush page %d of %d(file desc) during purge page.\n", used_frame->getPageId(), used_frame->file_desc_);
      return rc;
    }
  }
  printf("Successfully purge frame =%p, page %d of %d(file desc)\n", used_frame, used_frame->getPageId(), used_frame->file_desc_);
  frame_manager_.free(file_desc_, page_num, used_frame);
  return RE::SUCCESS;
}

RE DiskBufferPool::checkPageNum(int32_t page_num)
{
  if (page_num >= file_header_->page_count)
  {
    printf("Invalid pageNum:%d, file's name:%s\n", page_num, file_name_.c_str());
    return RE::FAIL;
  }
  if ((file_header_->bitmap[page_num / 8] & (1 << (page_num % 8))) == 0)
  {
    printf("Invalid pageNum:%d, file's name:%s\n", page_num, file_name_.c_str());
    return RE::FAIL;
  }
  return RE::SUCCESS;
}

RE DiskBufferPool::loadPage(int32_t page_num, Frame *frame)
{
  long long offset = ((long long)page_num) * sizeof(Page);
  if (lseek(file_desc_, offset, SEEK_SET) == -1)
  {
    printf("Failed to load page %s:%d, due to failed to lseek:%s.\n",
           file_name_.c_str(), page_num, strerror(errno));

    return RE::FAIL;
  }

  int ret = readn(file_desc_, &(frame->page_), sizeof(Page));
  if (ret != 0)
  {
    printf("Failed to load page %s:%d, due to failed to read data:%s, ret=%d, page count=%d",
           file_name_.c_str(), page_num, strerror(errno), ret, file_header_->allocated_pages);
    return RE::FAIL;
  }
  return RE::SUCCESS;
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

BufferPoolManager::BufferPoolManager()
{
  frame_manager_.initialize(MEM_POOL_ITEM_NUM);
}

BufferPoolManager &BufferPoolManager::getInstance()
{
  static BufferPoolManager instance;
  return instance;
}

void BufferPoolManager::initialize()
{
  frame_manager_.initialize(DEFAULT_ITEM_NUM_PER_POOL);
}

RE BufferPoolManager::createFile(const char *file_name)
{
  int fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, S_IREAD | S_IWRITE);
  if (fd < 0)
  {
    printf("Failed to create %s, due to %s.\n", file_name, strerror(errno));
    return RE::FAIL;
  }
  close(fd);
  fd = open(file_name, O_RDWR);
  if (fd < 0)
  {
    printf("Failed to open for readwrite %s, due to %s.\n", file_name, strerror(errno));
    return RE::FAIL;
  }
  Page page;
  memset(&page, 0, sizeof(Page));
  BPFileHeader *file_header = (BPFileHeader *)page.page_data_;
  file_header->allocated_pages = 1;
  file_header->page_count = 1;
  char *bitmap = file_header->bitmap;
  bitmap[0] |= 0x01;
  if (lseek(fd, 0, SEEK_SET) == -1)
  {
    printf("Failed to seek file %s to position 0, due to %s .\n", file_name, strerror(errno));
    close(fd);
    return RE::FAIL;
  }
  if (writen(fd, (char *)&page, sizeof(Page)) != 0)
  {
    printf("Failed to write header to file %s, due to %s.\n", file_name, strerror(errno));
    close(fd);
    return RE::FAIL;
  }
  close(fd);
  printf("Successfully create %s.\n", file_name);
  return RE::SUCCESS;
}

RE BufferPoolManager::openFile(const char *_file_name, DiskBufferPool *&_bp)
{
  std::string file_name(_file_name);
  if (buffer_pools_.find(file_name) != buffer_pools_.end())
  {
    printf("file already opened. file name=%s", _file_name);
    return RE::FAIL;
  }
  DiskBufferPool *bp = new DiskBufferPool(*this, frame_manager_);
  RE re = bp->openFile(_file_name);
  if (re != RE::SUCCESS)
  {
    printf("failed to open file name\n");
    delete bp;
    return re;
  }
  buffer_pools_.insert(std::pair<std::string, DiskBufferPool *>(file_name, bp));
  fd_buffer_pools_.insert(std::pair<int, DiskBufferPool *>(bp->getFileDesc(), bp));
  _bp = bp;
  return RE::SUCCESS;
}

RE BufferPoolManager::closeFile(const char *_file_name)
{
  std::string file_name(_file_name);
  auto iter = buffer_pools_.find(file_name);
  if (iter == buffer_pools_.end())
  {
    printf("file has not opened: %s", _file_name);
    return RE::FAIL;
  }
  int fd = iter->second->getFileDesc();
  fd_buffer_pools_.erase(fd);
  DiskBufferPool *bp = iter->second;
  buffer_pools_.erase(iter);
  delete bp;
  return RE::SUCCESS;
}

RE BufferPoolManager::flushPage(Frame &frame)
{
  int fd = frame.getFileDesc();
  auto iter = fd_buffer_pools_.find(fd);
  if (iter == fd_buffer_pools_.end())
  {
    printf("unknown buffer pool of fd %d\n", fd);
    return RE::FAIL;
  }
  DiskBufferPool *bp = iter->second;
  return bp->flushPage(frame);
}

void BufferPoolManager::destroy()
{
  std::unordered_map<std::string, DiskBufferPool *> tmp_bps;
  tmp_bps.swap(buffer_pools_);
  for (auto &iter : tmp_bps)
    delete iter.second;
}
