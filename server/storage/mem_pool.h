#pragma once
#include <stdio.h>
#include <list>
#include <set>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <assert.h>
#define MP_NAME_MAX_SIZE 20
#define DEFAULT_ITEM_NUM_PER_POOL 128
#define DEFAULT_POOL_NUM 1
// 接管new/delete
template <class t>
class MemoryPool
{
public:
    MemoryPool(const char *name);
    ~MemoryPool();
    bool initialize(bool is_dynamic = true, int pool_num = DEFAULT_POOL_NUM,
                    int item_num_per_pool = DEFAULT_ITEM_NUM_PER_POOL);
    void cleanUp();
    bool extend();
    t *alloc();
    void free(t *item);
    std::string toString();
    const std::string getName() const { return name_; }
    int getSize() const { return size_; }
    int getUsedSize();

private:
    bool is_dynamic_;
    std::recursive_mutex mutex_;
    int size_;
    std::string name_;
    // why list and set
    // pools_中的单位只是作为申请新空间的单位 本身所有pools_中的成员数组构成一整个pool作为整体活动
    std::list<t *> pools_;
    std::set<t *> used_;
    std::list<t *> free_;
    int item_num_per_pool_;
};

template <class t>
inline MemoryPool<t>::MemoryPool(const char *name) : name_(name)
{
    size_ = 0;
}

template <class t>
inline MemoryPool<t>::~MemoryPool()
{
}

template <class t>
inline bool MemoryPool<t>::initialize(bool is_dynamic, int pool_num, int item_num_per_pool)
{
    if (!pools_.empty())
    {
        printf("MemoryPool:pool is not empty,the memory pool is already initialized\n");
        return false;
    }
    if (pool_num <= 0 or item_num_per_pool <= 0)
    {
        printf("MemoryPool:invalid args:pool_num:%d item_num_per_pool%d\n", pool_num, item_num_per_pool);
    }
    item_num_per_pool_ = item_num_per_pool;
    is_dynamic_ =  true;
    for (int i = 0; i < pool_num;i++)
    {
        if (!extend())
        {
            cleanUp();
            return false;
        }
    }
    is_dynamic_ = is_dynamic;

    return true;
}

template <class t>
inline void MemoryPool<t>::cleanUp()
{
    free_.clear();
    used_.clear();
    mutex_.lock();
    for (auto it = pools_.begin(); it != pools_.end(); it++)
    {
        t *ptr = *it;
        delete[] ptr;
    }
    pools_.clear();
    mutex_.unlock();
}

template <class t>
inline bool MemoryPool<t>::extend()
{
    if (!is_dynamic_)
    {
        printf("MemoryPool:not a dynamic pool\n");
        return false;
    }
    mutex_.lock();
    t *new_items = new t[item_num_per_pool_];
    if (new_items == nullptr)
    {
        printf("MemoryPool:extend failed;number of items is %d\n", item_num_per_pool_);
        return false;
    }
    pools_.push_back(new_items);
    for (int i = 0; i < item_num_per_pool_; i++)
        free_.push_back(new_items[i]);
    mutex_.unlock();
    return true;
}

template <class t>
inline t *MemoryPool<t>::alloc()
{
    if (free_.empty())
    {
        if (!is_dynamic_)
            return nullptr;
        if (!extend())
            return nullptr;
    }
    mutex_.lock();
    t *res = free_.front();
    free_.pop_front();
    used_.insert(res);
    mutex_.unlock();
    return res;
}

template <class t>
inline void MemoryPool<t>::free(t *item)
{
}

template <class t>
inline std::string MemoryPool<t>::toString()
{
    std::stringstream ss;
    ss << "name:" << name_ << ","
       << "dyanmic:" << is_dynamic_ << ","
       << "size:" << size_ << ","
       << "pool_size:" << pools_.size() << ","
       << "used_size:" << used_.size() << ","
       << "free_size:" << free_.size();
    return ss.str();
}

template <class t>
inline int MemoryPool<t>::getUsedSize()
{
    mutex_.lock();
    int res = used_.size();
    mutex_.unlock();
    return res;
}
