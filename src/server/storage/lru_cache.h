#pragma once

#include <functional>
#include <unordered_set>

///@brief Hash= Pred= 是默认值的用法 不是using=那种
template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
class LruCache {
private:
    class ListNode {
    public:
        ListNode(const Key &key, const Value &value) : key_(key), value_(value) {}

    private:
        Key key_;
        Value value_;
        ListNode *prev_ = nullptr;
        ListNode *next_ = nullptr;

    private:
        friend class LruCache;
    };

    class PListNodeHasher {
    public:
        size_t operator()(ListNode *node) const;

    private:
        Hash hasher_;
    };

    class PListNodePredicator {
    public:
        bool operator()(ListNode *const node_1, ListNode *const node_2) const;

    private:
        Pred pred_;
    };

public:
    explicit LruCache(size_t reserve = 0);

    ~LruCache() { destroy(); }

    void destroy();

    [[nodiscard]] const size_t count() const { return searcher_.size(); }

    bool get(const Key &key, Value &value);

    void put(const Key &key, const Value &value);

    void remove(const Key &key);

    void foreach(std::function<bool(const Key &, const Value &)> func);

    void foreachReverse(std::function<bool(const Key &, const Value &)> func);

private:
    /// @brief 对该node进行了访问 需要移动其位置
    void lruTouch(ListNode *node);

    /// @brief 进入链表头
    void lruPush(ListNode *node);

    /// @brief 移出
    void lruRemove(ListNode *node);

private:
    std::unordered_set<ListNode *, PListNodeHasher, PListNodePredicator> searcher_;
    ListNode *lru_front_ = nullptr;
    ListNode *lru_tail_ = nullptr;
};

template<typename Key, typename Value, typename Hash, typename Pred>
inline size_t LruCache<Key, Value, Hash, Pred>::PListNodeHasher::operator()(ListNode *node) const {
    if (node == nullptr)
        return 0;
    return hasher_(node->key_);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline bool
LruCache<Key, Value, Hash, Pred>::PListNodePredicator::operator()(ListNode *const node_1,
                                                                  ListNode *const node_2) const {
    if (node_1 == node_2)
        return true;
    if (node_1 == nullptr || node_2 == nullptr)
        return false;
    return pred_(node_1->key_, node_2->key_);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline LruCache<Key, Value, Hash, Pred>::LruCache(size_t reserve) {
    if (reserve > 0)
        searcher_.reserve(reserve);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::destroy() {
    for (ListNode *node: searcher_)
        delete node;
    searcher_.clear();
    lru_front_ = nullptr;
    lru_tail_ = nullptr;
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline bool LruCache<Key, Value, Hash, Pred>::get(const Key &key, Value &value) {
    auto iter = searcher_.find((ListNode *) &key);
    if (iter == searcher_.end())
        return false;
    lruTouch(*iter);
    value = (*iter)->value_;
    return true;
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::put(const Key &key, const Value &value) {
    auto iter = searcher_.find((ListNode *) &key);
    if (iter != searcher_.end()) {
        ListNode *ln = *iter;
        ln->value_ = value;
        lruTouch(ln);
        return;
    }
    ListNode *ln = new ListNode(key, value);
    lruPush(ln);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::remove(const Key &key) {
    auto iter = searcher_.find((ListNode *) &key);
    if (iter != searcher_.end())
        lruRemove(*iter);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::foreach(std::function<bool(const Key &, const Value &)> func) {
    for (ListNode *node = lru_front_; node != nullptr; node = node->next_) {
        bool ret = func(node->key_, node->value_);
        if (!ret)
            break;
    }
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::foreachReverse(std::function<bool(const Key &, const Value &)> func) {
    for (ListNode *node = lru_tail_; node != nullptr; node = node->prev_) {
        bool ret = func(node->key_, node->value_);
        if (!ret)
            break;
    }
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::lruTouch(ListNode *node) {
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

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::lruPush(ListNode *node) {
    if (nullptr == lru_tail_)
        lru_tail_ = node;
    node->prev_ = nullptr;
    node->next_ = lru_front_;
    if (lru_front_ != nullptr)
        lru_front_->prev_ = node;
    lru_front_ = node;
    searcher_.insert(node);
}

template<typename Key, typename Value, typename Hash, typename Pred>
inline void LruCache<Key, Value, Hash, Pred>::lruRemove(ListNode *node) {
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