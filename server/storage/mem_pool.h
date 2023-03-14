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
template <typename T>
class MemoryPool
{
public:
    MemoryPool(const char *name);
    ~MemoryPool();
    bool initialize(bool is_dynamic = true, int pool_num = DEFAULT_POOL_NUM,
                    int item_num_per_pool = DEFAULT_ITEM_NUM_PER_POOL);
    void cleanUp();
    bool extend();
    T *alloc();
    void free(T *item);
    std::string toString();
    const std::string getName() const { return name_; }
    int getSize() const { return size_; }
    int getUsedSize();

private:
    bool is_dynamic_;
    std::recursive_mutex lock_;
    int size_;
    std::string name_;
    // why list and set
    // pools_中的单位只是作为申请新空间的单位 本身所有pools_中的成员数组构成一整个pool作为整体活动
    std::list<T *> pools_;
    std::set<T *> used_;
    std::list<T *> free_;
    int item_num_per_pool_;
};

template <class T>
inline MemoryPool<T>::MemoryPool(const char *name) : name_(name)
{
    size_ = 0;
}

template <class T>
inline MemoryPool<T>::~MemoryPool()
{
}

template <class T>
inline bool MemoryPool<T>::initialize(bool is_dynamic, int pool_num, int item_num_per_pool)
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

template <class T>
inline void MemoryPool<T>::cleanUp()
{
    free_.clear();
    used_.clear();
    lock_.lock();
    for (auto it = pools_.begin(); it != pools_.end(); it++)
    {
        T *ptr = *it;
        delete[] ptr;
    }
    pools_.clear();
    lock_.unlock();
}

template <class T>
inline bool MemoryPool<T>::extend()
{
    if (!is_dynamic_)
    {
        printf("MemoryPool:not a dynamic pool\n");
        return false;
    }
    lock_.lock();
    T *new_items = new T[item_num_per_pool_];
    if (new_items == nullptr)
    {
        printf("MemoryPool:extend failed;number of items is %d\n", item_num_per_pool_);
        return false;
    }
    pools_.push_back(new_items);
    for (int i = 0; i < item_num_per_pool_; i++)
        free_.push_back(new_items[i]);
    lock_.unlock();
    return true;
}

template <class T>
inline T *MemoryPool<T>::alloc()
{
    if (free_.empty())
    {
        if (!is_dynamic_)
            return nullptr;
        if (!extend())
            return nullptr;
    }
    lock_.lock();
    T *res = free_.front();
    free_.pop_front();
    used_.insert(res);
    lock_.unlock();
    return res;
}

template <class T>
inline void MemoryPool<T>::free(T *item)
{
    lock_.lock();
    size_t temp=used_.erase(item);
    if(temp==0){
        lock_.unlock();
        return;
    }
    free_.push_back(item);
    lock_.unlock();
}

template <class T>
inline std::string MemoryPool<T>::toString()
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

template <class T>
inline int MemoryPool<T>::getUsedSize()
{
    lock_.lock();
    int res = used_.size();
    lock_.unlock();
    return res;
}
