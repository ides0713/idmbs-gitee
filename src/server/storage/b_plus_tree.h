#pragma once
#include "../../common/common_defs.h"
#include "../common/re.h"
#include "../parse/parse_defs.h"
// #include "buffer_pool.h"
#include "record.h"
#include <functional>
#include <sstream>
#include <string.h>
#define EMPTY_RID_PAGE_ID -1
#define EMPTY_RID_SLOT_ID -1
class Frame;
class MemPoolItem;
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
    int32_t key_length;// attr length + sizeof(RecordId)
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
    static const int HEADER_SIZE = 12;
};
struct LeafIndexNode : public IndexNode {
public:
    int32_t prev_brother;
    int32_t next_brother;
    char array[0];

public:
    static const int HEADER_SIZE = IndexNode::HEADER_SIZE + 8;
};
struct InternalIndexNode : public IndexNode {
public:
    char array[0];

public:
    static const int HEADER_SIZE = IndexNode::HEADER_SIZE;
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
    const IndexFileHeader &header;
    int32_t page_id;
    IndexNode *node;
};
class LeafIndexNodeHandler : public IndexNodeHandler
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
class InternalIndexNodeHandler : public IndexNodeHandler
{
public:
    InternalIndexNodeHandler(const IndexFileHeader &header, Frame *frame);
    void InitEmpty();
    void CreateNewRoot(int32_t first_page_id, const char *key, int32_t page_num);
    void Insert(const char *key, int32_t page_id, const KeyComparator &comparator);
    Re MoveHalfTo(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    char *GetKeyAt(int index);
    int32_t GetValueAt(int index);
    /**
   * 返回指定子节点在当前节点中的索引
   */
    int GetValueIndex(int32_t page_id);
    void SetKeyAt(int index, const char *key);
    void Remove(int index);
    /**
   * 与Leaf节点不同，lookup返回指定key应该属于哪个子节点，返回这个子节点在当前节点中的索引
   * 如果想要返回插入位置，就提供 `insert_position` 参数
   * NOTE: 查找效率不高，你可以优化它吗?
   */
    int LookUp(const KeyComparator &comparator, const char *key, bool *found = nullptr,
               int *insert_position = nullptr) const;
    int GetMaxSize() const;
    int GetMinSize() const;
    Re MoveTo(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    Re MoveFirstToEnd(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    Re MoveLastToFront(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    Re MoveHalfTo(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool);
    bool IsValidate(const KeyComparator &comparator, DiskBufferPool *buffer_pool) const;
    friend std::string ToString(const InternalIndexNodeHandler &handler, const KeyPrinter &printer);

private:
    InternalIndexNode *internal_node_;

private:
    Re CopyFrom(const char *items, int num, DiskBufferPool *disk_buffer_pool);
    Re Append(const char *item, DiskBufferPool *bp);
    Re Preappend(const char *item, DiskBufferPool *bp);
    char *GetItemAt(int index) const;
    char *__GetKeyAt(int index) const;
    char *__GetValueAt(int index) const;
    int GetValueSize() const;
    int GetItemSize() const;
};
class BplusTreeHandler
{
public:
    BplusTreeHandler();
    /// @brief 此函数创建一个名为fileName的索引.attrType描述被索引属性的类型,attrLength描述被索引属性的长度
    Re Create(const char *file_name, AttrType attr_type, int attr_length, int internal_max_size = -1,
              int leaf_max_size = -1);
    ///@brief 打开名为fileName的索引文件.如果方法调用成功，则indexHandle为指向被打开的索引句柄的指针.索引句柄用于在索引中插入或删除索引项，也可用于索引的扫描
    Re Open(const char *file_name);
    ///@brief 关闭句柄indexHandle对应的索引文件
    Re Close();
    /**
   * 此函数向IndexHandle对应的索引中插入一个索引项。
   * 参数user_key指向要插入的属性值，参数rid标识该索引项对应的元组，
   * 即向索引中插入一个值为（user_key，rid）的键值对
   * @note 这里假设user_key的内存大小与attr_length 一致
   */
    Re InsertEntry(const char *user_key, const RecordId *rid);
    /**
   * 从IndexHandle句柄对应的索引中删除一个值为（*pData，rid）的索引项
   * @return RECORD_INVALID_KEY 指定值不存在
   * @note 这里假设user_key的内存大小与attr_length 一致
   */
    Re DeleteEntry(const char *user_key, const RecordId *rid);
    bool IsEmpty() const;
    /**
   * 获取指定值的record
   * @param key_len user_key的长度
   * @param rids  返回值，记录记录所在的页面号和slot
   */
    Re GetEntry(const char *user_key, int key_len, std::list<RecordId> &rids);
    Re Sync();
    /**
   * Check whether current B+ tree is invalid or not.
   * return true means current tree is valid, return false means current tree is invalid.
   * @return
   */
    bool IsValidateTree();
    Re PrintTree();
    Re PrintLeafs();

protected:
    DiskBufferPool *buffer_pool;
    bool header_dirty;
    IndexFileHeader file_header;
    KeyComparator key_comparator;
    KeyPrinter key_printer;
    MemPoolItem *mem_pool_item;

protected:
    Re FindLeaf(const char *key, Frame *&frame);
    Re LeftMostPage(Frame *&frame);
    Re RightMostPage(Frame *&frame);
    Re FindLeafInternal(const std::function<int32_t(InternalIndexNodeHandler &)> &child_page_getter, Frame *&frame);
    // Re InsertIntoParent(int32_t parent_page, Frame *left_frame, const char *pkey, Frame &right_frame);
    Re DeleteEntryInternal(Frame *leaf_frame, const char *key);
    // Re InsertIntoNewRoot(Frame *left_frame, const char *pkey, Frame &right_frame);
    template<typename IndexNodeHandlerType>
    Re Split(Frame *frame, Frame *&new_frame);
    template<typename IndexNodeHandlerType>
    Re CoalesceOrRedistribute(Frame *frame);
    template<typename IndexNodeHandlerType>
    Re Coalesce(Frame *neighbor_frame, Frame *frame, Frame *parent_frame, int index);
    template<typename IndexNodeHandlerType>
    Re Redistribute(Frame *neighbor_frame, Frame *frame, Frame *parent_frame, int index);
    Re InsertEntryIntoParent(Frame *frame, Frame *new_frame, const char *key);
    Re InsertEntryIntoLeafNode(Frame *frame, const char *key, const RecordId *rid);
    Re UpdateRootPageId();
    Re CreateNewTree(const char *key, const RecordId *rid);
    Re AdjustRoot(Frame *root_frame);

private:
    Re PrintLeaf(Frame *frame);
    Re PrintInternalNodeRecursive(Frame *frame);
    bool IsValidateLeafLink();
    bool IsValidateNodeRecursive(Frame *frame);
    char *MakeKey(const char *user_key, const RecordId &rid);
    void FreeKey(char *key);

private:
    friend class BplusTreeScanner;
    friend class BplusTreeTester;
};
class BplusTreeScanner
{
public:
    BplusTreeScanner(BplusTreeHandler &tree_handler);
    ~BplusTreeScanner();
    /**
   * 扫描指定范围的数据
   * @param left_user_key 扫描范围的左边界，如果是null，则没有左边界
   * @param left_len left_user_key 的内存大小(只有在变长字段中才会关注)
   * @param left_inclusive 左边界的值是否包含在内
   * @param right_user_key 扫描范围的右边界。如果是null，则没有右边界
   * @param right_len right_user_key 的内存大小(只有在变长字段中才会关注)
   * @param right_inclusive 右边界的值是否包含在内
   */
    Re Open(const char *left_user_key, int left_len, bool left_inclusive, const char *right_user_key, int right_len,
            bool right_inclusive);
    Re NextEntry(RecordId *rid);
    Re Close();

private:
    bool inited_ = false;
    BplusTreeHandler &tree_handler_;
    /// 使用左右叶子节点和位置来表示扫描的起始位置和终止位置
    /// 起始位置和终止位置都是有效的数据
    Frame *left_frame_ = nullptr;
    Frame *right_frame_ = nullptr;
    int iter_index_ = -1;
    int end_index_ = -1;// use -1 for end of scan
private:
    /**
   * 如果key的类型是CHARS, 扩展或缩减user_key的大小刚好是schema中定义的大小
   */
    Re FixUserKey(const char *user_key, int key_len, bool want_greater, char **fixed_key, bool *should_inclusive);
};