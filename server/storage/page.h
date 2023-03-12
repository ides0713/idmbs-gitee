#pragma once
#include <stdint.h>
#define BP_INVALID_PAGE_NUM (-1)
#define BP_PAGE_SIZE (1 << 14)
#define BP_PAGE_DATA_SIZE (BP_PAGE_SIZE - sizeof(int32_t))
struct Page
{
    int32_t page_id;
    char page_data[BP_PAGE_SIZE];
};
