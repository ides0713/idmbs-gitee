#include "mem_pool.h"
#include "cstring"
#include "string"
MemPoolItem::MemPoolItem(const char *tag) : name(tag) {
    size = 0;
    pthread_mutexattr_t mutexatr;
    pthread_mutexattr_init(&mutexatr);
    pthread_mutexattr_settype(&mutexatr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, &mutexatr);
}
MemPoolItem::~MemPoolItem() {
    Cleanup();
    pthread_mutex_destroy(&mutex);
}
int MemPoolItem::Init(int item_size, bool dynamic, int pool_num, int item_num_per_pool) {
    if (!pools.empty()) {
        DebugPrint("MemPoolItem:memory pool has been initialized, but still begin to be initialized,name:%s.\n",
                   name.c_str());
        return 0;
    }
    if (item_size <= 0 or pool_num <= 0 or item_num_per_pool <= 0) {
        DebugPrint("MemPoolItem:invalid arguments, item_size:%d, pool_num:%d, item_num_per_pool:%d,name:%s.\n",
                   item_size, pool_num, item_num_per_pool, name.c_str());
        return -1;
    }
    this->item_size = item_size;
    this->item_num_per_pool = item_num_per_pool;
    // in order to init memory pool, enable dynamic here
    this->dynamic = true;
    for (int i = 0; i < pool_num; i++) {
        if (Extend() < 0) {
            Cleanup();
            return -1;
        }
    }
    this->dynamic = dynamic;
    DebugPrint("MemPoolItem:extend one pool,size:%d,item_size:%d,item_num_per_pool:%d,name:%s.\n", size, item_size,
               item_num_per_pool, name.c_str());
    return 0;
}
void MemPoolItem::Cleanup() {
    if (pools.empty()) {
        DebugPrint("MemPoolItem:begin to do cleanup, but there is no memory pool,name:%s\n", name.c_str());
        return;
    }
    pthread_mutex_lock(&this->mutex);
    used.clear();
    frees.clear();
    size = 0;
    for (std::list<void *>::iterator iter = pools.begin(); iter != pools.end(); iter++) {
        void *pool = *iter;
        free(pool);
    }
    pools.clear();
    pthread_mutex_unlock(&this->mutex);
    DebugPrint("MemPoolItem:successfully do cleanup,name:%s.\n", name.c_str());
}
int MemPoolItem::Extend() {
    if (!dynamic) {
        DebugPrint("MemPoolItem:disable dynamic extend memory pool,but begin to extend,name:%s\n", name.c_str());
        return -1;
    }
    pthread_mutex_lock(&mutex);
    void *pool = malloc(item_num_per_pool * item_size);
    if (pool == nullptr) {
        pthread_mutex_unlock(&this->mutex);
        DebugPrint("MemPoolItem:failed to extend memory pool,size:%d,item_num_per_pool:%d,name:%s.\n", size,
                   item_num_per_pool, name.c_str());
        return -1;
    }
    pools.push_back(pool);
    size += item_num_per_pool;
    for (int i = 0; i < item_num_per_pool; i++) {
        char *item = (char *) pool + i * item_size;
        frees.push_back((void *) item);
    }
    pthread_mutex_unlock(&mutex);
    DebugPrint("MemPoolItem:extend one pool,size:%d,item_size:%d,item_num_per_pool:%d,name:%s.\n", size, item_size,
               item_num_per_pool, name.c_str());
    return 0;
}
void *MemPoolItem::Alloc() {
    pthread_mutex_lock(&this->mutex);
    if (frees.empty()) {
        if (!dynamic) {
            pthread_mutex_unlock(&mutex);
            return nullptr;
        }
        if (Extend() < 0) {
            pthread_mutex_unlock(&mutex);
            return nullptr;
        }
    }
    void *buffer = frees.front();
    frees.pop_front();
    used.insert(buffer);
    pthread_mutex_unlock(&this->mutex);
    memset(buffer, 0, item_size);//memset item_size or sizeof(item_size)
    return buffer;
}
void MemPoolItem::Free(void *item) {
    pthread_mutex_lock(&mutex);
    size_t num = used.erase(item);
    if (num == 0) {
        pthread_mutex_unlock(&mutex);
        DebugPrint("MemPoolItem:no entry of %p in %s.\n", item, name.c_str());
        return;
    }
    frees.push_back(item);
    pthread_mutex_unlock(&mutex);
}
bool MemPoolItem::IsUsed(void *item) {
    pthread_mutex_lock(&mutex);
    auto it = used.find(item);
    pthread_mutex_unlock(&mutex);
    return it != used.end();
}
std::string MemPoolItem::ToString() {
    std::stringstream ss;
    ss << "name:" << this->name << ","
       << "dyanmic:" << this->dynamic << ","
       << "size:" << this->size << ","
       << "pool_size:" << this->pools.size() << ","
       << "used_size:" << this->used.size() << ","
       << "free_size:" << this->frees.size();
    return ss.str();
}
int MemPoolItem::GetUsedNum() {
    pthread_mutex_lock(&mutex);
    auto num = used.size();
    pthread_mutex_unlock(&mutex);
    return num;
}
