#pragma once
#include "page.h"
#define BP_FILE_HDR_SIZE (sizeof(BPFileSubHeader))
struct BPFileHeader {
  int32_t page_count;        //! 当前文件一共有多少个页面
  int32_t allocated_pages;   //! 已经分配了多少个页面
  char    bitmap[0];         //! 页面分配位图, 第0个页面(就是当前页面)，总是1

  /**
   * 能够分配的最大的页面个数，即bitmap的字节数 乘以8
   */
  static const int MAX_PAGE_NUM = (BP_PAGE_DATA_SIZE - sizeof(page_count) - sizeof(allocated_pages)) * 8;
};