#pragma once
#include <functional>
#include <unordered_set>
///@brief Hash= Pred= 是默认值的用法 不是using=那种
template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
class LRUCache
{
private:
    class ListNode
    {
    public:
        ListNode(const Key &key, const Value &value) : key_(key), value_(value) {}

    private:
        Key key_;
        Value value_;
        ListNode *prev_ = nullptr;
        ListNode *next_ = nullptr;

    private:
        friend class LRUCache;
    };
    class PListNodeHasher
    {
    public:
        size_t operator()(ListNode *node) const;

    private:
        Hash hasher_;
    };
    class PListNodePredicator
    {
    public:
        bool operator()(ListNode *const node1, ListNode *const node2) const;

    private:
        Pred pred_;
    };

public:
    LRUCache(size_t reserve = 0);
    ~LRUCache() { destroy(); }
    void destroy();
    size_t count() const { return searcher_.size(); }
    bool get(const Key &key, Value &value);
    void put(const Key &key, const Value &value);
    void remove(const Key &key);
    void foreach (std::function<bool(const Key &, const Value &)> func);
    void foreachReverse(std::function<bool(const Key &, const Value &)> func);

private:
    /// @brief 对该node进行了访问 需要移动其位置
    void LRUTouch(ListNode *node);
    /// @brief 进入链表头
    void LRUPush(ListNode *node);
    /// @brief 移出
    void LRURemove(ListNode *node);
private:
    std::unordered_set<ListNode *, PListNodeHasher, PListNodePredicator> searcher_;
    ListNode *lru_front_ = nullptr;
    ListNode *lru_tail_ = nullptr;
};

template <typename Key, typename Value, typename Hash, typename Pred>
inline size_t LRUCache<Key, Value, Hash, Pred>::PListNodeHasher::operator()(ListNode *node) const
{
    if (node == nullptr)
        return 0;
    return hasher_(node->key_);
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline bool LRUCache<Key, Value, Hash, Pred>::PListNodePredicator::operator()(ListNode *const node1, ListNode *const node2) const
{
    if (node1 == node2)
        return true;
    if (node1 == nullptr || node2 == nullptr)
        return false;
    return pred_(node1->key_, node2->key_);
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline LRUCache<Key, Value, Hash, Pred>::LRUCache(size_t reserve = 0)
{
    if (reserve > 0)
        searcher_.reserve(reserve);
}
template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::destroy()
{
    for (ListNode *node : searcher_)
        delete node;
    searcher_.clear();
    lru_front_ = nullptr;
    lru_tail_ = nullptr;
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline bool LRUCache<Key, Value, Hash, Pred>::get(const Key &key, Value &value)
{
    auto iter = searcher_.find((ListNode *)&key);
    if (iter == searcher_.end())
        return false;
    LRUTouch(*iter);
    value = (*iter)->value_;
    return true;
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::put(const Key &key, const Value &value)
{
    auto iter = searcher_.find((ListNode *)&key);
    if (iter != searcher_.end())
    {
        ListNode *ln = *iter;
        ln->value_ = value;
        LRUTouch(ln);
        return;
    }
    ListNode *ln = new ListNode(key, value);
    LRUPush(ln);
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::remove(const Key &key)
{
    auto iter = searcher_.find((ListNode *)&key);
    if (iter != searcher_.end())
        LRURemove(*iter);
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::foreach (std::function<bool(const Key &, const Value &)> func)
{
    for (ListNode *node = lru_front_; node != nullptr; node = node->next_)
    {
        bool ret = func(node->key_, node->value_);
        if (!ret)
            break;
    }
}
template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::foreachReverse(std::function<bool(const Key &, const Value &)> func)
{
    for (ListNode *node = lru_tail_; node != nullptr; node = node->prev_)
    {
        bool ret = func(node->key_, node->value_);
        if (!ret)
            break;
    }
}
template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::LRUTouch(ListNode *node)
{
    if (nullptr == node->prev_)
        return;
    node->prev_->next_ = node->next_;
    if (node->next_ != nullptr)
        node->next_->prev_ = node->prev_;
    else
        lru_tail_ = node->prev_;
    node->prev_ = nullptr;
    node->next_ = lru_front_;
    if (lru_front_ != nullptr)
        lru_front_->prev_ = node;
    lru_front_ = node;
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::LRUPush(ListNode *node)
{
    if (nullptr == lru_tail_)
        lru_tail_ = node;
    node->prev_ = nullptr;
    node->next_ = lru_front_;
    if (lru_front_ != nullptr)
        lru_front_->prev_ = node;
    lru_front_ = node;
    searcher_.insert(node);
}

template <typename Key, typename Value, typename Hash, typename Pred>
inline void LRUCache<Key, Value, Hash, Pred>::LRURemove(ListNode *node)
{
    if (node->prev_ != nullptr)
        node->prev_->next_ = node->next_;
    if (node->next_ != nullptr)
        node->next_->prev_ = node->prev_;
    if (lru_front_ == node)
        lru_front_ = node->next_;
    if (lru_tail_ == node)
        lru_tail_ = node->prev_;
    searcher_.erase(node);
    delete node;
}