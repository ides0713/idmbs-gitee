#pragma once
#include "../../common/common_defs.h"
#include "../common/re.h"
#include "../parse/parse_defs.h"
// #include "buffer_pool.h"
#include <functional>
#include <sstream>
#include <string.h>
#define EMPTY_RID_PAGE_ID -1
#define EMPTY_RID_SLOT_ID -1
class Frame;
class DiskBufferPool;
class AttrComparator
{
public:
    void Init(AttrType type, int length);
    int GetAttrLength() const { return attr_length_; }
    int operator()(const char *v1, const char *v2) const;

private:
    AttrType attr_type_;
    int attr_length_;
};
class KeyComparator
{
public:
    void Init(AttrType type, int length);
    const AttrComparator &GetAttrComparator() const { return attr_comparator_; }
    int operator()(const char *v1, const char *v2) const;

private:
    AttrComparator attr_comparator_;
};
class AttrPrinter
{
public:
    void Init(AttrType type, int length);
    int GetAttrLength() const { return attr_length_; }
    std::string operator()(const char *v) const;

private:
    AttrType attr_type_;
    int attr_length_;
};
class KeyPrinter
{
public:
    void Init(AttrType type, int length);
    const AttrPrinter &GetAttrPrinter() const { return attr_printer_; }
    std::string operator()(const char *v) const;

private:
    AttrPrinter attr_printer_;
};
struct IndexFileHeader {
public:
    int32_t root_page_id;
    int32_t internal_max_size;
    int32_t leaf_max_size;
    int32_t attr_length;
    int32_t key_length;// attr length + sizeof(RID)
    AttrType attr_type;

public:
    IndexFileHeader();
    const std::string ToString();
};
#define RECORD_RESERVER_PAIR_NUM 2
/// @brief common header of index node
struct IndexNode {
public:
    bool is_leaf;
    int key_num;
    int32_t parent;

public:
    static constexpr int HEADER_SIZE = 12;
};
struct LeafIndexNode : public IndexNode {
public:
    int32_t prev_brother;
    int32_t next_brother;
    char array[0];

public:
    static constexpr int HEADER_SIZE = IndexNode::HEADER_SIZE + 8;
};
struct InternalIndexNode : public IndexNode {
public:
    char array[0];

public:
    static constexpr int HEADER_SIZE = IndexNode::HEADER_SIZE;
};
class IndexNodeHandler
{
public:
    IndexNodeHandler(const IndexFileHeader &header, Frame *frame);
    void InitEmpty(bool is_leaf);
    bool IsLeaf() const;
    int GetKeySize() const;
    int GetValueSize() const;
    int GetItemSize() const;
    void IncreaseSize(int n);
    int GetSize() const;
    void SetParentPageId(int32_t page_id);
    int32_t GetParentPageId() const;
    int32_t GetParentId() const;
    int32_t GetPageId() const;
    bool IsValidate() const;
    friend std::string ToString(const IndexNodeHandler &handler);

protected:
    const IndexFileHeader &header_;
    int32_t page_id_;
    IndexNode *node_;
};
class LeafIndexNodeHandler
{
public:
    LeafIndexNodeHandler(const IndexFileHeader &header, Frame *frame);
    void InitEmpty();
    void SetNextPage(int32_t page_id);
    void SetPrevPage(int32_t page_id);
    int32_t GetNextPage() const;
    int32_t GetPrevPage() const;
    char *GetKeyAt(int index);
    char *GetValueAt(int index);
    int LookUp(const KeyComparator &comparator, const char *key, bool *found = nullptr) const;
    void Insert(int index, const char *key, const char *value);
    void Remove(int index);
    int Remove(const char *key, const KeyComparator &comparator);
    Re MoveHalfTo(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    Re MoveFirstToEnd(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    Re MoveLastToFront(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    ///@brief move all items to left pages
    Re MoveTo(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    int GetMaxSize() const;
    int GetMinSize() const;
    bool IsValidate(const KeyComparator &comparator, DiskBufferPool *buffer_pool) const;
    friend std::string ToString(const LeafIndexNodeHandler &handler, const KeyPrinter &printer);

private:
    LeafIndexNode *leaf_node_;

private:
    char *GetItemAt(int index) const;
    char *__GetKeyAt(int index) const;
    char *__GetValueAt(int index) const;
    void Append(const char *item);
    void Preappend(const char *item);
};
//   /**
//    * 查找指定key的插入位置(注意不是key本身)
//    * 如果key已经存在，会设置found的值
//    * NOTE: 当前lookup的实现效率非常低，你是否可以优化它?
//    */
//   int lookup(const KeyComparator &comparator, const char *key, bool *found = nullptr) const;
//   void insert(int index, const char *key, const char *value);
//   void remove(int index);
//   int  remove(const char *key, const KeyComparator &comparator);
//   RC   move_half_to(LeafIndexNodeHandler &other, DiskBufferPool *bp);
//   RC   move_first_to_end(LeafIndexNodeHandler &other, DiskBufferPool *disk_buffer_pool);
//   RC   move_last_to_front(LeafIndexNodeHandler &other, DiskBufferPool *bp);
//   /**
//    * move all items to left page
//    */
//   RC move_to(LeafIndexNodeHandler &other, DiskBufferPool *bp);
//   int max_size() const;
//   int min_size() const;
//   bool validate(const KeyComparator &comparator, DiskBufferPool *bp) const;
//   friend std::string to_string(const LeafIndexNodeHandler &handler, const KeyPrinter &printer);
// private:
//   char *__item_at(int index) const;
//   char *__key_at(int index) const;
//   char *__value_at(int index) const;
//   void append(const char *item);
//   void preappend(const char *item);
// private:
//   LeafIndexNode *leaf_node_;
// };
class InternalIndexNodeHandler
{
};