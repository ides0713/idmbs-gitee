#pragma once
#include "../../common/common_defs.h"
#include <cassert>
#include <cstdio>
#include <list>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#define MP_NAME_MAX_SIZE 20
#define DEFAULT_ITEM_NUM_PER_POOL 128
#define DEFAULT_POOL_NUM 1
// 接管new/delete
template<typename T>
class MemoryPool
{
public:
    explicit MemoryPool(const char *name);
    ~MemoryPool();
    bool Init(bool is_dynamic = true, int pool_num = DEFAULT_POOL_NUM,
              int item_num_per_pool = DEFAULT_ITEM_NUM_PER_POOL);
    void CleanUp();
    bool Extend();
    T *Alloc();
    void Free(T *item);
    std::string ToString();
    [[nodiscard]] std::string GetName() const { return name_; }
    [[nodiscard]] int GetSize() const { return size_; }
    [[nodiscard]] int GetUsedSize();

private:
    bool is_dynamic_;
    std::recursive_mutex lock_;
    int size_;
    std::string name_;
    std::list<T *> pools_;
    std::set<T *> used_;
    std::list<T *> free_;
    int item_num_per_pool_;
};
template<class T>
inline MemoryPool<T>::MemoryPool(const char *name) : name_(name) {
    size_ = 0, is_dynamic_ = false, item_num_per_pool_ = 0;
}
template<class T>
inline MemoryPool<T>::~MemoryPool() {
    CleanUp();
}
template<class T>
inline bool MemoryPool<T>::Init(bool is_dynamic, int pool_num, int item_num_per_pool) {
    if (!pools_.empty()) {
        DebugPrint("MemoryPool:pool is not empty,the memory pool is already initialized\n");
        return true;
    }
    if (pool_num <= 0 or item_num_per_pool <= 0) {
        DebugPrint("MemoryPool:invalid args:pool_num:%d item_num_per_pool%d\n", pool_num, item_num_per_pool);
        return false;
    }
    item_num_per_pool_ = item_num_per_pool;
    is_dynamic_ = true;
    for (int i = 0; i < pool_num; i++) {
        if (!Extend()) {
            CleanUp();
            return false;
        }
    }
    is_dynamic_ = is_dynamic;
    return true;
}
template<class T>
inline void MemoryPool<T>::CleanUp() {
    free_.clear();
    used_.clear();
    lock_.lock();
    for (auto it = pools_.begin(); it != pools_.end(); it++) {
        T *ptr = *it;
        delete[] ptr;
    }
    pools_.clear();
    lock_.unlock();
}
template<class T>
inline bool MemoryPool<T>::Extend() {
    if (!is_dynamic_) {
        DebugPrint("MemoryPool:not a dynamic pool\n");
        return false;
    }
    lock_.lock();
    T *new_items = new T[item_num_per_pool_];
    if (new_items == nullptr) {
        DebugPrint("MemoryPool:extend failed;number of items is %d\n", item_num_per_pool_);
        return false;
    }
    pools_.push_back(new_items);
    for (int i = 0; i < item_num_per_pool_; i++)
        free_.push_back(new_items + i);
    lock_.unlock();
    return true;
}
template<class T>
inline T *MemoryPool<T>::Alloc() {
    if (free_.empty()) {
        if (!is_dynamic_)
            return nullptr;
        if (!Extend())
            return nullptr;
    }
    lock_.lock();
    T *res = free_.front();
    free_.pop_front();
    used_.insert(res);
    lock_.unlock();
    return res;
}
template<class T>
inline void MemoryPool<T>::Free(T *item) {
    lock_.lock();
    size_t temp = used_.erase(item);
    if (temp == 0) {
        lock_.unlock();
        return;
    }
    free_.push_back(item);
    lock_.unlock();
}
template<class T>
inline std::string MemoryPool<T>::ToString() {
    std::stringstream ss;
    ss << "name:" << name_ << ","
       << "dyanmic:" << is_dynamic_ << ","
       << "size:" << size_ << ","
       << "pool_size:" << pools_.size() << ","
       << "used_size:" << used_.size() << ","
       << "free_size:" << free_.size();
    return ss.str();
}
template<class T>
inline int MemoryPool<T>::GetUsedSize() {
    lock_.lock();
    int res = used_.size();
    lock_.unlock();
    return res;
}
class MemPoolItem
{
public:
    MemPoolItem(const char *tag);
    virtual ~MemPoolItem();
    /**
   * init memory pool, the major job is to alloc memory for memory pool
   * @param pool_num, memory pool's number
   * @param item_num_per_pool, how many items per pool.
   * @return
   */
    int Init(int item_size, bool dynamic = true, int pool_num = DEFAULT_POOL_NUM,
             int item_num_per_pool = DEFAULT_ITEM_NUM_PER_POOL);
    /**
   * Do cleanup job for memory pool
   */
    void Cleanup();
    /**
   * If dynamic has been set, extend current memory pool,
   */
    int Extend();
    /**
   * Alloc one frame from memory Pool
   * @return
   */
    void *Alloc();
    /**
   * Free one item, the resouce will return to memory Pool
   * @param item
   */
    void Free(void *item);
    /**
   * Check whether this item has been used before.
   * @param item
   * @return
   */
    bool IsUsed(void *item);
    std::string ToString();
    const std::string GetName() const { return name; }
    bool IsDynamic() const { return dynamic; }
    int GetSize() const { return size; }
    int GetItemSize() const { return item_size; }
    int GetItemNumPerPool() const { return item_num_per_pool; }
    int GetUsedNum();

protected:
    pthread_mutex_t mutex;
    std::string name;
    bool dynamic;
    int size;
    int item_size;
    int item_num_per_pool;
    std::list<void *> pools;
    std::set<void *> used;
    std::list<void *> frees;
};