#include "b_plus_tree.h"
#include "../common/global_managers.h"
#include "../common/lower_bound.h"
#include "buffer_pool.h"
#include "record.h"
#include "../common/persist_file_io_handler.h"
#define FIRST_INDEX_PAGE 1
/// @brief internal node [key(attr+RecordId),page_id] 
int CalcInternalPageCapacity(int attr_length) {
    int item_size = attr_length + sizeof(RecordId) + sizeof(int32_t);
    int capacity = (BP_PAGE_DATA_SIZE - InternalIndexNode::HEADER_SIZE) / item_size;
    return capacity;
}
/// @brief leaf node [key(attr+RecordId),RecordId]
int CalcLeafPageCapacity(int attr_length) {
    int item_size = attr_length + sizeof(RecordId) + sizeof(RecordId);
    int capacity = (BP_PAGE_DATA_SIZE - LeafIndexNode::HEADER_SIZE) / item_size;
    return capacity;
}
void AttrComparator::Init(AttrType type, int length) {
    attr_type_ = type;
    attr_length_ = length;
}
int AttrComparator::operator()(const char *v1, const char *v2) const {
    switch (attr_type_) {
        case AttrType::Ints:
            return CompareInt((void *) v1, (void *) v2);
        case AttrType::Dates:
            assert(false);
        case AttrType::Floats:
            return CompareFloat((void *) v1, (void *) v2);
        case AttrType::Chars:
            return CompareString((void *) v1, attr_length_, (void *) v2, attr_length_);
        default:
            DebugPrint("AttrComparator:unknown attr type\n");
            assert(false);
    }
}
void KeyComparator::Init(AttrType type, int length) {
    attr_comparator_.Init(type, length);
}
int KeyComparator::operator()(const char *v1, const char *v2) const {
    int result = attr_comparator_(v1, v2);
    if (result != 0)
        return result;
    const RecordId *rid1 = reinterpret_cast<const RecordId *>(v1 + attr_comparator_.GetAttrLength());
    const RecordId *rid2 = reinterpret_cast<const RecordId *>(v2 + attr_comparator_.GetAttrLength());
    return RecordId::Compare(rid1, rid2);
}
void AttrPrinter::Init(AttrType type, int length) {
    attr_type_ = type;
    attr_length_ = length;
}
std::string AttrPrinter::operator()(const char *v) const {
    switch (attr_type_) {
        case AttrType::Ints:
            return std::to_string(*(int *) v);
        case AttrType::Dates:
            assert(false);
        case AttrType::Floats:
            return std::to_string(*(float *) v);
        case AttrType::Chars: {
            std::string str;
            for (int i = 0; i < attr_length_; i++) {
                if (v[i] == 0)
                    break;
                str.push_back(v[i]);
            }
            return str;
        } break;
        default:
            DebugPrint("KeyComparator:unknown attr type\n");
            assert(false);
    }
}
void KeyPrinter::Init(AttrType type, int length) {
    attr_printer_.Init(type, length);
}
std::string KeyPrinter::operator()(const char *v) const {
    std::stringstream ss;
    ss << "{key:" << attr_printer_(v) << ",";
    const RecordId *rid = reinterpret_cast<const RecordId *>(v + attr_printer_.GetAttrLength());
    ss << "rid:{" << rid->ToString() << "}}";
    return ss.str();
}
IndexFileHeader::IndexFileHeader() {
    memset(this, 0, sizeof(IndexFileHeader));
    root_page_id = BP_INVALID_PAGE_ID;
}
const std::string IndexFileHeader::ToString() {
    std::stringstream ss;
    ss << "attr_length:" << attr_length << ","
       << "key_length:" << key_length << ","
       << "attr_type:" << attr_type << ","
       << "root_page_id:" << root_page_id << ","
       << "internal_max_size:" << internal_max_size << ","
       << "leaf_max_size:" << leaf_max_size << ";";
    return ss.str();
}
IndexNodeHandler::IndexNodeHandler(const IndexFileHeader &__header, Frame *frame)
    : header(__header), page_id(frame->GetPageId()), node(reinterpret_cast<IndexNode *>(frame->GetPageData())) {
}
void IndexNodeHandler::InitEmpty(bool is_leaf) {
    node->is_leaf = is_leaf;
    node->key_num = 0;
    node->parent = BP_INVALID_PAGE_ID;
}
bool IndexNodeHandler::IsLeaf() const {
    return node->is_leaf;
}
int IndexNodeHandler::GetKeySize() const {
    return header.key_length;
}
int IndexNodeHandler::GetValueSize() const {
    return sizeof(RecordId);
}
int IndexNodeHandler::GetItemSize() const {
    return GetKeySize() + GetValueSize();
}
void IndexNodeHandler::IncreaseSize(int n) {
    node->key_num += n;
}
int IndexNodeHandler::GetSize() const {
    return node->key_num;
}
void IndexNodeHandler::SetParentPageId(int32_t __page_id) {
    node->parent = __page_id;
}
int32_t IndexNodeHandler::GetParentPageId() const {
    return node->parent;
}
int32_t IndexNodeHandler::GetPageId() const {
    return page_id;
}
bool IndexNodeHandler::IsValidate() const {
    if (GetParentPageId() == BP_INVALID_PAGE_ID) {
        if (GetSize() < 1) {
            DebugPrint("IndexNodeHandler:root page has no item\n");
            return false;
        }
        if (!IsLeaf() and GetSize() < 2) {
            DebugPrint("IndexNodeHandler:root page internal node has less than 2 child. size=%d\n", GetSize());
            return false;
        }
    }
    return true;
}
std::string ToString(const IndexNodeHandler &handler) {
    std::stringstream ss;
    ss << "PageId:" << handler.GetPageId() << ","
       << "is_leaf:" << handler.IsLeaf() << ","
       << "key_num:" << handler.GetSize() << ","
       << "parent:" << handler.GetParentPageId() << ",";
    return ss.str();
}
LeafIndexNodeHandler::LeafIndexNodeHandler(const IndexFileHeader &__header, Frame *frame)
    : IndexNodeHandler(__header, frame), leaf_node_(reinterpret_cast<LeafIndexNode *>(frame->GetPageData())) {
}
void LeafIndexNodeHandler::InitEmpty() {
    IndexNodeHandler::InitEmpty(true);
    leaf_node_->prev_brother = BP_INVALID_PAGE_ID;
    leaf_node_->next_brother = BP_INVALID_PAGE_ID;
}
void LeafIndexNodeHandler::SetNextPage(int32_t page_id) {
    leaf_node_->next_brother = page_id;
}
void LeafIndexNodeHandler::SetPrevPage(int32_t page_id) {
    leaf_node_->prev_brother = page_id;
}
int32_t LeafIndexNodeHandler::GetNextPage() const {
    return leaf_node_->next_brother;
}
int32_t LeafIndexNodeHandler::GetPrevPage() const {
    return leaf_node_->prev_brother;
}
char *LeafIndexNodeHandler::GetKeyAt(int index) {
    assert(index >= 0 and index < GetSize());
    return __GetKeyAt(index);
}
char *LeafIndexNodeHandler::GetValueAt(int index) {
    assert(index >= 0 and index < GetSize());
    return __GetValueAt(index);
}
int LeafIndexNodeHandler::GetMaxSize() const {
    return header.leaf_max_size;
}
int LeafIndexNodeHandler::GetMinSize() const {
    return header.leaf_max_size - header.leaf_max_size / 2;
}
int LeafIndexNodeHandler::LookUp(const KeyComparator &comparator, const char *key, bool *found /* = nullptr */) const {
    const int size = GetSize();
    BinaryIterator<char> iter_begin(GetItemSize(), __GetKeyAt(0));
    BinaryIterator<char> iter_end(GetItemSize(), __GetKeyAt(size));
    BinaryIterator<char> iter = FindLowerBound(iter_begin, iter_end, key, comparator, found);
    return iter - iter_begin;
}
void LeafIndexNodeHandler::Insert(int index, const char *key, const char *value) {
    if (index < GetSize())
        memmove(GetItemAt(index + 1), GetItemAt(index), (GetSize() - index) * GetItemSize());
    memmove(GetItemAt(index), key, GetKeySize());
    memmove(GetItemAt(index) + GetKeySize(), value, GetValueSize());
    IncreaseSize(1);
}
void LeafIndexNodeHandler::Remove(int index) {
    assert(index >= 0 and index < GetSize());
    if (index < GetSize() - 1)
        memmove(GetItemAt(index), GetItemAt(index + 1), (GetSize() - index - 1) * GetItemSize());
    IncreaseSize(-1);
}
int LeafIndexNodeHandler::Remove(const char *key, const KeyComparator &comparator) {
    bool found = false;
    int index = LookUp(comparator, key, &found);
    if (found) {
        this->Remove(index);
        return 1;
    }
    return 0;
}
Re LeafIndexNodeHandler::MoveHalfTo(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    const int size = this->GetSize();
    const int move_index = size / 2;
    memmove(other.GetItemAt(0), this->GetItemAt(move_index), GetItemSize() * (size - move_index));
    other.IncreaseSize(size - move_index);
    this->IncreaseSize(-(size - move_index));
    return Re::Success;
}
Re LeafIndexNodeHandler::MoveFirstToEnd(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    other.Append(GetItemAt(0));
    if (GetSize() >= 1)
        memmove(GetItemAt(0), GetItemAt(1), (GetSize() - 1) * GetItemSize());
    IncreaseSize(-1);
    return Re::Success;
}
Re LeafIndexNodeHandler::MoveLastToFront(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    other.Preappend(GetItemAt(GetSize() - 1));
    IncreaseSize(-1);
    return Re::Success;
}
/**
 * move all items to left page
 */
Re LeafIndexNodeHandler::MoveTo(LeafIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    memmove(other.GetItemAt(other.GetSize()), this->GetItemAt(0), this->GetSize() * GetItemSize());
    other.IncreaseSize(this->GetSize());
    this->IncreaseSize(-this->GetSize());
    other.SetNextPage(this->GetNextPage());
    int32_t next_right_page_id = this->GetNextPage();
    if (next_right_page_id != BP_INVALID_PAGE_ID) {
        Frame *next_right_frame;
        Re r = buffer_pool->GetPage(next_right_page_id, &next_right_frame);
        if (r != Re::Success) {
            DebugPrint("LeafIndexNodeHandler:failed to fetch next right page. page id:%d. r=%d:%s\n",
                       next_right_page_id, r, StrRe(r));
            return r;
        }
        LeafIndexNodeHandler next_right_node(header, next_right_frame);
        next_right_node.SetPrevPage(other.GetPageId());
        next_right_frame->DirtyMark();
        buffer_pool->UnpinPage(next_right_frame);
    }
    return Re::Success;
}
void LeafIndexNodeHandler::Append(const char *item) {
    memmove(GetItemAt(GetSize()), item, GetItemSize());
    IncreaseSize(1);
}
void LeafIndexNodeHandler::Preappend(const char *item) {
    if (GetSize() > 0)
        memmove(GetItemAt(1), GetItemAt(0), GetSize() * GetItemSize());
    memmove(GetItemAt(0), item, GetItemSize());
    IncreaseSize(1);
}
char *LeafIndexNodeHandler::GetItemAt(int index) const {
    return leaf_node_->array + (index * GetItemSize());
}
char *LeafIndexNodeHandler::__GetKeyAt(int index) const {
    return GetItemAt(index);
}
char *LeafIndexNodeHandler::__GetValueAt(int index) const {
    return GetItemAt(index) + GetKeySize();
}
std::string ToString(const LeafIndexNodeHandler &handler, const KeyPrinter &printer) {
    std::stringstream ss;
    ss << ToString(reinterpret_cast<const IndexNodeHandler &>(handler)) << ",prev page:" << handler.GetPrevPage()
       << ",next page:" << handler.GetNextPage();
    ss << ",values=[" << printer(handler.__GetKeyAt(0));
    for (int i = 1; i < handler.GetSize(); i++)
        ss << "," << printer(handler.__GetKeyAt(i));
    ss << "]";
    return ss.str();
}
bool LeafIndexNodeHandler::IsValidate(const KeyComparator &comparator, DiskBufferPool *buffer_pool) const {
    bool result = IndexNodeHandler::IsValidate();
    if (!result)
        return false;
    const int node_size = GetSize();
    for (int i = 1; i < node_size; i++) {
        if (comparator(__GetKeyAt(i - 1), __GetKeyAt(i)) >= 0) {
            DebugPrint("LeafIndexNodeHandler:page id = %d, invalid key order. id1=%d,id2=%d, this=%s\n", GetPageId(),
                       i - 1, i, ToString(*this).c_str());
            return false;
        }
    }
    int32_t parent_page_id = this->GetParentPageId();
    if (parent_page_id == BP_INVALID_PAGE_ID)
        return true;
    Frame *parent_frame;
    Re r = buffer_pool->GetPage(parent_page_id, &parent_frame);
    if (r != Re::Success) {
        DebugPrint("LeafIndexNodeHandler:failed to fetch parent page. page id=%d, r=%d:%s\n", parent_page_id, r,
                   StrRe(r));
        return false;
    }
    InternalIndexNodeHandler parent_node(header, parent_frame);
    int index_in_parent = parent_node.GetValueIndex(this->GetPageId());
    if (index_in_parent < 0) {
        DebugPrint("LeafIndexNodeHandler:invalid leaf node. cannot find index in parent. this page id=%d, parent page "
                   "id=%d\n",
                   this->GetPageId(), parent_page_id);
        buffer_pool->UnpinPage(parent_frame);
        return false;
    }
    if (0 != index_in_parent) {
        int cmp_result = comparator(__GetKeyAt(0), parent_node.GetKeyAt(index_in_parent));
        if (cmp_result < 0) {
            DebugPrint("LeafIndexNodeHandler:invalid leaf node. first item should be greate than or equal to parent "
                       "item. this page id=%d, parent page id=%d, index in parent=%d\n",
                       this->GetPageId(), parent_node.GetPageId(), index_in_parent);
            buffer_pool->UnpinPage(parent_frame);
            return false;
        }
    }
    if (index_in_parent < parent_node.GetSize() - 1) {
        int cmp_result = comparator(__GetKeyAt(GetSize() - 1), parent_node.GetKeyAt(index_in_parent + 1));
        if (cmp_result >= 0) {
            DebugPrint("LeafIndexNodeHandler:invalid leaf node. last item should be less than the item at the first "
                       "after item in parent."
                       "this page id=%d, parent page id=%d, parent item to compare=%d\n",
                       this->GetPageId(), parent_node.GetPageId(), index_in_parent + 1);
            buffer_pool->UnpinPage(parent_frame);
            return false;
        }
    }
    buffer_pool->UnpinPage(parent_frame);
    return true;
}
/////////////////////////////////////////////////
InternalIndexNodeHandler::InternalIndexNodeHandler(const IndexFileHeader &header, Frame *frame)
    : IndexNodeHandler(header, frame), internal_node_(reinterpret_cast<InternalIndexNode *>(frame->GetPageData())) {
}
std::string ToString(const InternalIndexNodeHandler &node, const KeyPrinter &printer) {
    std::stringstream ss;
    ss << ToString(reinterpret_cast<const IndexNodeHandler &>(node));
    ss << ",children:["
       << "{key:" << printer(node.__GetKeyAt(0)) << ","
       << "value:" << *reinterpret_cast<int32_t *>(node.__GetValueAt(0)) << "}";
    for (int i = 1; i < node.GetSize(); i++) {
        ss << ",{key:" << printer(node.__GetKeyAt(i)) << ",value:" << *reinterpret_cast<int32_t *>(node.__GetValueAt(i))
           << "}";
    }
    ss << "]";
    return ss.str();
}
void InternalIndexNodeHandler::InitEmpty() {
    IndexNodeHandler::InitEmpty(false);
}
void InternalIndexNodeHandler::CreateNewRoot(int32_t first_page_id, const char *key, int32_t page_id) {
    memset(__GetKeyAt(0), 0, GetKeySize());
    memmove(__GetValueAt(0), &first_page_id, GetValueSize());
    memmove(GetItemAt(1), key, GetKeySize());
    memmove(__GetValueAt(1), &page_id, GetValueSize());
    IncreaseSize(2);
}
// /**
//  * insert one entry
//  * the entry to be inserted will never at the first slot.
//  * the right child page after split will always have bigger keys.
//  */
void InternalIndexNodeHandler::Insert(const char *key, int32_t page_id, const KeyComparator &comparator) {
    int insert_position = -1;
    LookUp(comparator, key, nullptr, &insert_position);
    if (insert_position < GetSize())
        memmove(GetItemAt(insert_position + 1), GetItemAt(insert_position),
                (GetSize() - insert_position) * GetItemSize());
    memmove(GetItemAt(insert_position), key, GetKeySize());
    memmove(__GetValueAt(insert_position), &page_id, GetValueSize());
    IncreaseSize(1);
}
Re InternalIndexNodeHandler::MoveHalfTo(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    const int size = this->GetSize();
    const int move_index = size / 2;
    Re r = other.CopyFrom(this->GetItemAt(move_index), size - move_index, buffer_pool);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to copy item to new node. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    IncreaseSize(-(size - move_index));
    return Re::Success;
}
int InternalIndexNodeHandler::GetMaxSize() const {
    return header.internal_max_size;
}
int InternalIndexNodeHandler::GetMinSize() const {
    return header.internal_max_size - header.internal_max_size / 2;
}
int InternalIndexNodeHandler::LookUp(const KeyComparator &comparator, const char *key, bool *found /* = nullptr */,
                                     int *insert_position /*= nullptr */) const {
    const int size = this->GetSize();
    if (size == 0) {
        if (insert_position)
            *insert_position = 1;
        if (found)
            *found = false;
        return 0;
    }
    BinaryIterator<char> iter_begin(GetItemSize(), __GetKeyAt(1));
    BinaryIterator<char> iter_end(GetItemSize(), __GetKeyAt(size));
    BinaryIterator<char> iter = FindLowerBound(iter_begin, iter_end, key, comparator, found);
    int ret = static_cast<int>(iter - iter_begin) + 1;
    if (insert_position)
        *insert_position = ret;
    if (ret >= size or comparator(key, __GetKeyAt(ret)) < 0)
        return ret - 1;
    return ret;
}
char *InternalIndexNodeHandler::GetKeyAt(int index) {
    assert(index >= 0 and index < GetSize());
    return __GetKeyAt(index);
}
void InternalIndexNodeHandler::SetKeyAt(int index, const char *key) {
    assert(index >= 0 and index < GetSize());
    memmove(__GetKeyAt(index), key, GetKeySize());
}
int32_t InternalIndexNodeHandler::GetValueAt(int index) {
    assert(index >= 0 and index < GetSize());
    return *reinterpret_cast<int32_t *>(__GetValueAt(index));
}
int InternalIndexNodeHandler::GetValueIndex(int32_t page_id) {
    for (int i = 0; i < GetSize(); i++)
        if (page_id == *reinterpret_cast<int32_t *>(__GetValueAt(i)))
            return i;
    return -1;
}
void InternalIndexNodeHandler::Remove(int index) {
    assert(index >= 0 and index < GetSize());
    if (index < GetSize() - 1)
        memmove(GetItemAt(index), GetItemAt(index + 1), (GetSize() - index - 1) * GetItemSize());
    IncreaseSize(-1);
}
Re InternalIndexNodeHandler::MoveTo(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    Re r = other.CopyFrom(GetItemAt(0), GetSize(), buffer_pool);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to copy items to other node. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    IncreaseSize(-this->GetSize());
    return Re::Success;
}
Re InternalIndexNodeHandler::MoveFirstToEnd(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    Re r = other.Append(GetItemAt(0), buffer_pool);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to append item to others.\n");
        return r;
    }
    if (GetSize() >= 1)
        memmove(GetItemAt(0), GetItemAt(1), (GetSize() - 1) * GetItemSize());
    IncreaseSize(-1);
    return Re::Success;
}
Re InternalIndexNodeHandler::MoveLastToFront(InternalIndexNodeHandler &other, DiskBufferPool *buffer_pool) {
    Re r = other.Preappend(GetItemAt(GetSize() - 1), buffer_pool);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to preappend to others\n");
        return r;
    }
    IncreaseSize(-1);
    return Re::Success;
}
/**
 * copy items from other node to self's right
 */
Re InternalIndexNodeHandler::CopyFrom(const char *items, int num, DiskBufferPool *buffer_pool) {
    memmove(GetItemAt(this->GetSize()), items, num * GetItemSize());
    Re r;
    int32_t this_page_id = this->GetPageId();
    Frame *frame = nullptr;
    for (int i = 0; i < num; i++) {
        const int32_t page_id = *reinterpret_cast<const int32_t *>((items + i * GetItemSize()) + GetKeySize());
        r = buffer_pool->GetPage(page_id, &frame);
        if (r != Re::Success) {
            DebugPrint("InternalIndexNodeHandler:failed to set child's page id. child page id:%d, this page id=%d, "
                       "r=%d:%s\n",
                       page_id, this_page_id, r, StrRe(r));
            return r;
        }
        IndexNodeHandler child_node(header, frame);
        child_node.SetParentPageId(this_page_id);
        frame->DirtyMark();
        buffer_pool->UnpinPage(frame);
    }
    IncreaseSize(num);
    return Re::Success;
}
Re InternalIndexNodeHandler::Append(const char *item, DiskBufferPool *buffer_pool) {
    return this->CopyFrom(item, 1, buffer_pool);
}
Re InternalIndexNodeHandler::Preappend(const char *item, DiskBufferPool *buffer_pool) {
    int32_t child_page_id = *(int32_t *) (item + GetKeySize());
    Frame *frame = nullptr;
    Re r = buffer_pool->GetPage(child_page_id, &frame);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to fetch child page. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    IndexNodeHandler child_node(header, frame);
    child_node.SetParentPageId(this->GetPageId());
    frame->DirtyMark();
    buffer_pool->UnpinPage(frame);
    if (this->GetSize() > 0)
        memmove(GetItemAt(1), GetItemAt(0), this->GetSize() * GetItemSize());
    memmove(GetItemAt(0), item, GetItemSize());
    IncreaseSize(1);
    return Re::Success;
}
char *InternalIndexNodeHandler::GetItemAt(int index) const {
    return internal_node_->array + (index * GetItemSize());
}
char *InternalIndexNodeHandler::__GetKeyAt(int index) const {
    return GetItemAt(index);
}
char *InternalIndexNodeHandler::__GetValueAt(int index) const {
    return GetItemAt(index) + GetKeySize();
}
int InternalIndexNodeHandler::GetValueSize() const {
    return sizeof(int32_t);
}
int InternalIndexNodeHandler::GetItemSize() const {
    return GetKeySize() + this->GetValueSize();
}
bool InternalIndexNodeHandler::IsValidate(const KeyComparator &comparator, DiskBufferPool *buffer_pool) const {
    bool result = IndexNodeHandler::IsValidate();
    if (!result)
        return false;
    const int node_size = GetSize();
    for (int i = 2; i < node_size; i++) {
        if (comparator(__GetKeyAt(i - 1), __GetKeyAt(i)) >= 0) {
            DebugPrint("page id = %d, invalid key order. id1=%d,id2=%d, this=%s\n", GetPageId(), i - 1, i,
                       ToString(*this).c_str());
            return false;
        }
    }
    for (int i = 0; result and i < node_size; i++) {
        int32_t page_id = *(int32_t *) __GetValueAt(i);
        if (page_id < 0)
            DebugPrint("InternalIndexNodeHandler:this page id=%d, got invalid child page. page id=%d\n",
                       this->GetPageId(), page_id);
        else {
            Frame *child_frame;
            Re r = buffer_pool->GetPage(page_id, &child_frame);
            if (r != Re::Success)
                DebugPrint("InternalIndexNodeHandler:failed to fetch child page while validate internal page. page "
                           "id=%d, r=%d:%s\n",
                           page_id, r, StrRe(r));
            else {
                IndexNodeHandler child_node(header, child_frame);
                if (child_node.GetParentPageId() != this->GetPageId()) {
                    DebugPrint("InternalIndexNodeHandler:child's parent page id is invalid. child page id=%d, parent "
                               "page id=%d, this page id=%d\n",
                               child_node.GetPageId(), child_node.GetParentPageId(), this->GetPageId());
                    result = false;
                }
                buffer_pool->UnpinPage(child_frame);
            }
        }
    }
    if (!result)
        return result;
    const int32_t parent_page_id = this->GetParentPageId();
    if (parent_page_id == BP_INVALID_PAGE_ID)
        return result;
    Frame *parent_frame;
    Re r = buffer_pool->GetPage(parent_page_id, &parent_frame);
    if (r != Re::Success) {
        DebugPrint("InternalIndexNodeHandler:failed to fetch parent page. page id=%d, r=%d:%s\n", parent_page_id, r,
                   StrRe(r));
        return false;
    }
    InternalIndexNodeHandler parent_node(header, parent_frame);
    int index_in_parent = parent_node.GetValueIndex(this->GetPageId());
    if (index_in_parent < 0) {
        DebugPrint("InternalIndexNodeHandler:invalid internal node. cannot find index in parent. this page id=%d, "
                   "parent page id=%d\n",
                   this->GetPageId(), parent_page_id);
        buffer_pool->UnpinPage(parent_frame);
        return false;
    }
    if (0 != index_in_parent) {
        int cmp_result = comparator(__GetKeyAt(1), parent_node.GetKeyAt(index_in_parent));
        if (cmp_result < 0) {
            DebugPrint("InternalIndexNodeHandler:invalid internal node. the second item should be greate than or equal "
                       "to parent item. this page id=%d, parent page id=%d, index in parent=%d\n",
                       this->GetPageId(), parent_node.GetPageId(), index_in_parent);
            buffer_pool->UnpinPage(parent_frame);
            return false;
        }
    }
    if (index_in_parent < parent_node.GetSize() - 1) {
        int cmp_result = comparator(__GetKeyAt(GetSize() - 1), parent_node.GetKeyAt(index_in_parent + 1));
        if (cmp_result >= 0) {
            DebugPrint("InternalIndexNodeHandler:invalid internal node. last item should be less than the item at the "
                       "first after item in parent.this page id=%d, parent page id=%d, parent item to compare=%d\n",
                       this->GetPageId(), parent_node.GetPageId(), index_in_parent + 1);
            buffer_pool->UnpinPage(parent_frame);
            return false;
        }
    }
    buffer_pool->UnpinPage(parent_frame);
    return result;
}
BplusTreeHandler::BplusTreeHandler() : buffer_pool(nullptr), header_dirty(false), mem_pool_item(nullptr) {
}
Re BplusTreeHandler::Create(const char *file_name, AttrType attr_type, int attr_length, int internal_max_size,
                            int leaf_max_size) {
    GlobalBufferPoolManager &bpm = GlobalManagers::GetGlobalBufferPoolManager();
    Re r = bpm.CreateFile(file_name);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to create file. file name=%s,r=%d:%s\n", file_name, r, StrRe(r));
        return r;
    }
    DebugPrint("BplusTreeHandler:successfully create index file:%s\n", file_name);
    DiskBufferPool *bp = nullptr;
    r = bpm.OpenFile(file_name, bp);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to open file.file name=%s,r=%d:%s\n", file_name, r, StrRe(r));
        return r;
    }
    DebugPrint("BplusTreeHandler:successfully open index file %s.\n", file_name);
    Frame *header_frame = nullptr;
    r = bp->AllocatePage(&header_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to allocate header page for bplus tree.r=%d:%s\n", r, StrRe(r));
        bpm.CloseFile(file_name);
        return r;
    }
    if (header_frame->GetPageId() != FIRST_INDEX_PAGE) {
        DebugPrint("BplusTreeHandler:header page id should be %d but got %d.is it a new file : %s\n", FIRST_INDEX_PAGE,
                   header_frame->GetPageId(), file_name);
        bpm.CloseFile(file_name);
        return Re::Internal;
    }
    if (internal_max_size < 0)
        internal_max_size = CalcInternalPageCapacity(attr_length);
    if (leaf_max_size < 0)
        leaf_max_size = CalcLeafPageCapacity(attr_length);
    char *pdata = header_frame->GetPageData();
    IndexFileHeader *file_header = reinterpret_cast<IndexFileHeader *>(pdata);
    file_header->attr_length = attr_length;
    file_header->key_length = attr_length + sizeof(RecordId);
    file_header->attr_type = attr_type;
    file_header->internal_max_size = internal_max_size;
    file_header->leaf_max_size = leaf_max_size;
    file_header->root_page_id = BP_INVALID_PAGE_ID;
    header_frame->DirtyMark();
    buffer_pool = bp;
    memmove(&this->file_header, pdata, sizeof(IndexFileHeader));
    header_dirty = false;
    bp->UnpinPage(header_frame);
    mem_pool_item = new MemPoolItem(file_name);//index of table has its own memory pool
    if (mem_pool_item->Init(this->file_header.key_length) < 0) {
        DebugPrint("BplusTreeHandler:failed to init memory pool for index %s\n", file_name);
        Close();
        return Re::NoMem;
    }
    key_comparator.Init(file_header->attr_type, file_header->attr_length);
    key_printer.Init(file_header->attr_type, file_header->attr_length);
    DebugPrint("BplusTreeHandler:successfully create index %s\n", file_name);
    return Re::Success;
}
Re BplusTreeHandler::Open(const char *file_name) {
    if (buffer_pool != nullptr) {
        DebugPrint("BplusTreeHandler:%s has been opened before index.open.\n", file_name);
        return Re::RecordOpened;
    }
    GlobalBufferPoolManager &bpm = GlobalManagers::GetGlobalBufferPoolManager();
    DiskBufferPool *bp = nullptr;
    Re r = bpm.OpenFile(file_name, bp);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to open file name=%s,r=%d:%s\n", file_name, r, StrRe(r));
        return r;
    }
    Frame *frame = nullptr;
    r = bp->GetPage(FIRST_INDEX_PAGE, &frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to get first page file name=%s,r=%d:%s\n", file_name, r, StrRe(r));
        bpm.CloseFile(file_name);
        return r;
    }
    char *pdata = frame->GetPageData();
    memmove(&file_header, pdata, sizeof(IndexFileHeader));
    header_dirty = false;
    buffer_pool = bp;
    mem_pool_item = new MemPoolItem(file_name);
    if (mem_pool_item->Init(file_header.key_length) < 0) {
        DebugPrint("BplusTreeHandler:failed to init memory pool for index %s\n", file_name);
        Close();
        return Re::NoMem;
    }
    // close old page_handle
    buffer_pool->UnpinPage(frame);
    key_comparator.Init(file_header.attr_type, file_header.attr_length);
    key_printer.Init(file_header.attr_type, file_header.attr_length);
    DebugPrint("BplusTreeHandler:successfully open index %s\n", file_name);
    return Re::Success;
}
Re BplusTreeHandler::Close() {
    if (buffer_pool != nullptr) {
        std::string file_name=buffer_pool->GetFileName();
        GlobalBufferPoolManager & bpm=GlobalManagers::GetGlobalBufferPoolManager();
        bpm.CloseFile(file_name.c_str());
        PersistFileIoHandler p;
        p.OpenFile(file_name.c_str());
        Re r=p.RemoveFile();
        if(r!=Re::Success){
            DebugPrint("BplusTreeHandler:close index failed,delete file '%s' failed\n",file_name.c_str());
            return r;
        }
        delete mem_pool_item;
        mem_pool_item = nullptr;
    }
    buffer_pool = nullptr;
    return Re::Success;
}
Re BplusTreeHandler::InsertEntry(const char *user_key, const RecordId *rid) {
    if (user_key == nullptr or rid == nullptr) {
        DebugPrint("BplusTreeHandler:invalid arguments, key is empty or rid is empty\n");
        return Re::InvalidArgument;
    }
    char *key = MakeKey(user_key, *rid);
    if (key == nullptr) {
        DebugPrint("BplusTreeHandler:failed to alloc memory for key.\n");
        return Re::NoMem;
    }
    if (IsEmpty()) {
        Re r = CreateNewTree(key, rid);
        mem_pool_item->Free(key);
        return r;
    }
    Frame *frame = nullptr;
    Re r = FindLeaf(key, frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to find leaf %s. r=%d:%s\n", rid->ToString().c_str(), r, StrRe(r));
        mem_pool_item->Free(key);
        return r;
    }
    r = InsertEntryIntoLeafNode(frame, key, rid);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to insert into leaf of index, rid:%s\n", rid->ToString().c_str());
        buffer_pool->UnpinPage(frame);
        mem_pool_item->Free(key);
        return r;
    }
    mem_pool_item->Free(key);
    DebugPrint("BplusTreeHandler:insert entry success\n");
    return Re::Success;
}
Re BplusTreeHandler::DeleteEntry(const char *user_key, const RecordId *rid) {
    char *key = reinterpret_cast<char *>(mem_pool_item->Alloc());
    if (key == nullptr) {
        DebugPrint("BplusTreeHandler:failed to alloc memory for key. size=%d\n", file_header.key_length);
        return Re::NoMem;
    }
    memmove(key, user_key, file_header.attr_length);
    memmove(key + file_header.attr_length, rid, sizeof(*rid));
    Frame *leaf_frame = nullptr;
    Re r = FindLeaf(key, leaf_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to find leaf page. r=%d:%s\n", r, StrRe(r));
        mem_pool_item->Free(key);
        return r;
    }
    r = DeleteEntryInternal(leaf_frame, key);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to delete index\n");
        mem_pool_item->Free(key);
        return r;
    }
    mem_pool_item->Free(key);
    return Re::Success;
}
bool BplusTreeHandler::IsEmpty() const {
    return file_header.root_page_id == BP_INVALID_PAGE_ID;
}
Re BplusTreeHandler::GetEntry(const char *user_key, int key_len, std::list<RecordId> &rids) {
    BplusTreeScanner scanner(*this);
    Re r = scanner.Open(user_key, key_len, true, user_key, key_len, true);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to open scanner.r=%d:%s\n", r, StrRe(r));
        return r;
    }
    RecordId rid;
    while ((r = scanner.NextEntry(&rid)) == Re::Success)
        rids.push_back(rid);
    scanner.Close();
    if (r != Re::RecordEof)
        DebugPrint("BplusTreeHandler:scanner return error.r=%d:%s\n", r, StrRe(r));
    else
        r = Re::Success;
    return r;
}
Re BplusTreeHandler::Sync() {
    return buffer_pool->FlushAllPages();
}
bool BplusTreeHandler::IsValidateTree() {
    if (IsEmpty())
        return true;
    Frame *frame = nullptr;
    Re r = buffer_pool->GetPage(file_header.root_page_id, &frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch root page.page id=%d,r=%d:%s\n", file_header.root_page_id, r,
                   StrRe(r));
        return r;
    }
    if (!IsValidateNodeRecursive(frame) or !IsValidateLeafLink()) {
        DebugPrint("BplusTreeHandler:Current B+ Tree is invalid\n");
        PrintTree();
        return false;
    }
    DebugPrint("BplusTreeHandler:current tree is valid!\n");
    return true;
}
Re BplusTreeHandler::PrintTree() {
    if (buffer_pool == nullptr) {
        DebugPrint("BplusTreeHandler:index hasn't been created or opened, fail to print\n");
        return Re::Success;
    }
    if (IsEmpty()) {
        DebugPrint("BplusTreeHandler:tree is empty\n");
        return Re::Success;
    }
    Frame *frame = nullptr;
    int32_t page_id = file_header.root_page_id;
    Re r = buffer_pool->GetPage(page_id, &frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch page.page id=%d,r=%d:%s\n", page_id, r, StrRe(r));
        return r;
    }
    IndexNodeHandler node(file_header, frame);
    if (node.IsLeaf())
        r = PrintLeaf(frame);
    else
        r = PrintInternalNodeRecursive(frame);
    return r;
}
Re BplusTreeHandler::PrintLeafs() {
    if (IsEmpty()) {
        DebugPrint("BplusTreeHandler:empty tree\n");
        return Re::Success;
    }
    Frame *frame = nullptr;
    Re r = LeftMostPage(frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to get left most page. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    while (frame->GetPageId() != BP_INVALID_PAGE_ID) {
        LeafIndexNodeHandler leaf_node(file_header, frame);
        DebugPrint("BplusTreeHandler:leaf info: %s\n", ToString(leaf_node, key_printer).c_str());
        int32_t next_page_id = leaf_node.GetNextPage();
        buffer_pool->UnpinPage(frame);
        if (next_page_id == BP_INVALID_PAGE_ID)
            break;
        r = buffer_pool->GetPage(next_page_id, &frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to get next page. page id=%d, r=%d:%s\n", next_page_id, r, StrRe(r));
            return r;
        }
    }
    return Re::Success;
}
Re BplusTreeHandler::FindLeaf(const char *key, Frame *&frame) {
    return FindLeafInternal(
            [&](InternalIndexNodeHandler &internal_node) {
                return internal_node.GetValueAt(internal_node.LookUp(key_comparator, key));
            },
            frame);
}
Re BplusTreeHandler::LeftMostPage(Frame *&frame) {
    return FindLeafInternal([&](InternalIndexNodeHandler &internal_node) { return internal_node.GetValueAt(0); },
                            frame);
}
Re BplusTreeHandler::RightMostPage(Frame *&frame) {
    return FindLeafInternal(
            [&](InternalIndexNodeHandler &internal_node) {
                return internal_node.GetValueAt(internal_node.GetSize() - 1);
            },
            frame);
}
Re BplusTreeHandler::FindLeafInternal(const std::function<int32_t(InternalIndexNodeHandler &)> &child_page_getter,
                                      Frame *&frame) {
    if (IsEmpty())
        return Re::Empty;
    Re r = buffer_pool->GetPage(file_header.root_page_id, &frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch root page.page id=%d,r=%d:%s\n", file_header.root_page_id, r,
                   StrRe(r));
        return r;
    }
    IndexNode *node = reinterpret_cast<IndexNode *>(frame->GetPageData());
    while (!node->is_leaf) {
        InternalIndexNodeHandler internal_node(file_header, frame);
        int32_t page_id = child_page_getter(internal_node);
        buffer_pool->UnpinPage(frame);
        r = buffer_pool->GetPage(page_id, &frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to load page page_id:%d\n", page_id);
            return r;
        }
        node = reinterpret_cast<IndexNode *>(frame->GetPageData());
    }
    return Re::Success;
}
Re BplusTreeHandler::DeleteEntryInternal(Frame *leaf_frame, const char *key) {
    LeafIndexNodeHandler leaf_index_node(file_header, leaf_frame);
    const int remove_count = leaf_index_node.Remove(key, key_comparator);
    if (remove_count == 0) {
        DebugPrint("BplusTreeHandler:no data to remove\n");
        buffer_pool->UnpinPage(leaf_frame);
        return Re::RecordRecordNotExist;
    }
    // leaf_index_node.validate(key_comparator_, disk_buffer_pool_, file_id_);
    leaf_frame->DirtyMark();
    if (leaf_index_node.GetSize() >= leaf_index_node.GetMinSize()) {
        buffer_pool->UnpinPage(leaf_frame);
        return Re::Success;
    }
    return CoalesceOrRedistribute<LeafIndexNodeHandler>(leaf_frame);
}
template<typename IndexNodeHandlerType>
Re BplusTreeHandler::Split(Frame *frame, Frame *&new_frame) {
    IndexNodeHandlerType old_node(file_header, frame);
    // add a new node
    Re r = buffer_pool->AllocatePage(&new_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to split index page due to failed to allocate page,r=%d:%s\n", r, StrRe(r));
        return r;
    }
    IndexNodeHandlerType new_node(file_header, new_frame);
    new_node.InitEmpty();
    new_node.SetParentPageId(old_node.GetParentPageId());
    old_node.MoveHalfTo(new_node, buffer_pool);
    frame->DirtyMark();
    new_frame->DirtyMark();
    return Re::Success;
}
template<typename IndexNodeHandlerType>
Re BplusTreeHandler::CoalesceOrRedistribute(Frame *frame) {
    IndexNodeHandlerType index_node(file_header, frame);
    if (index_node.GetSize() >= index_node.GetMinSize()) {
        buffer_pool->UnpinPage(frame);
        return Re::Success;
    }
    const int32_t parent_page_id = index_node.GetParentPageId();
    if (parent_page_id == BP_INVALID_PAGE_ID) {
        // this is the root page
        if (index_node.GetSize() > 1)
            buffer_pool->UnpinPage(frame);
        else// adjust the root node
            AdjustRoot(frame);
        return Re::Success;
    }
    Frame *parent_frame = nullptr;
    Re r = buffer_pool->GetPage(parent_page_id, &parent_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch parent page.page id=%d,r=%d:%s\n", parent_page_id, r, StrRe(r));
        buffer_pool->UnpinPage(frame);
        return r;
    }
    InternalIndexNodeHandler parent_index_node(file_header, parent_frame);
    int index = parent_index_node.LookUp(key_comparator, index_node.GetKeyAt(index_node.GetSize() - 1));
    if (parent_index_node.GetValueAt(index) != frame->GetPageId()) {
        DebugPrint("BplusTreeHandler:lookup return an invalid value.index=%d,this page id=%d,but got %d\n", index,
                   frame->GetPageId(), parent_index_node.GetValueAt(index));
    }
    int32_t neighbor_page_id;
    if (index == 0)
        neighbor_page_id = parent_index_node.GetValueAt(1);
    else
        neighbor_page_id = parent_index_node.GetValueAt(index - 1);
    Frame *neighbor_frame = nullptr;
    r = buffer_pool->GetPage(neighbor_page_id, &neighbor_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch neighbor page.page id=%d,r=%d:%s\n", neighbor_page_id, r,
                   StrRe(r));
        buffer_pool->UnpinPage(frame);
        buffer_pool->UnpinPage(parent_frame);
        return r;
    }
    IndexNodeHandlerType neighbor_node(file_header, neighbor_frame);
    if (index_node.GetSize() + neighbor_node.GetSize() > index_node.GetMaxSize())
        r = Redistribute<IndexNodeHandlerType>(neighbor_frame, frame, parent_frame, index);
    else
        r = Coalesce<IndexNodeHandlerType>(neighbor_frame, frame, parent_frame, index);
    return r;
}
template<typename IndexNodeHandlerType>
Re BplusTreeHandler::Coalesce(Frame *neighbor_frame, Frame *frame, Frame *parent_frame, int index) {
    IndexNodeHandlerType neighbor_node(file_header, neighbor_frame);
    IndexNodeHandlerType node(file_header, frame);
    InternalIndexNodeHandler parent_node(file_header, parent_frame);
    Frame *left_frame = nullptr, *right_frame = nullptr;
    if (index == 0) {
        // neighbor node is at right
        left_frame = frame;
        right_frame = neighbor_frame;
        index++;
    } else {
        left_frame = neighbor_frame;
        right_frame = frame;
        // neighbor is at left
    }
    IndexNodeHandlerType left_node(file_header, left_frame);
    IndexNodeHandlerType right_node(file_header, right_frame);
    parent_node.Remove(index);
    Re r = right_node.MoveTo(left_node, buffer_pool);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to move right node to left.r=%d:%s\n", r, StrRe(r));
        return r;
    }
    if (left_node.IsLeaf()) {
        LeafIndexNodeHandler left_leaf_node(file_header, left_frame), right_leaf_node(file_header, right_frame);
        left_leaf_node.SetNextPage(right_leaf_node.GetNextPage());
        int32_t next_right_page_id = right_leaf_node.GetNextPage();
        if (next_right_page_id != BP_INVALID_PAGE_ID) {
            Frame *next_right_frame = nullptr;
            r = buffer_pool->GetPage(next_right_page_id, &next_right_frame);
            if (r != Re::Success) {
                DebugPrint("BplusTreeHandler:failed to fetch next right page. page id:%d. r=%d:%s\n",
                           next_right_page_id, r, StrRe(r));
                buffer_pool->UnpinPage(frame);
                buffer_pool->UnpinPage(neighbor_frame);
                buffer_pool->UnpinPage(parent_frame);
                return r;
            }
            LeafIndexNodeHandler next_right_node(file_header, next_right_frame);
            next_right_node.SetPrevPage(left_node.GetPageId());
            buffer_pool->UnpinPage(next_right_frame);
        }
    }
    int32_t right_page_id = right_frame->GetPageId();
    buffer_pool->UnpinPage(left_frame);
    buffer_pool->UnpinPage(right_frame);
    buffer_pool->DisposePage(right_page_id);
    return CoalesceOrRedistribute<InternalIndexNodeHandler>(parent_frame);
}
template<typename IndexNodeHandlerType>
Re BplusTreeHandler::Redistribute(Frame *neighbor_frame, Frame *frame, Frame *parent_frame, int index) {
    InternalIndexNodeHandler parent_node(file_header, parent_frame);
    IndexNodeHandlerType neighbor_node(file_header, neighbor_frame);
    IndexNodeHandlerType node(file_header, frame);
    if (neighbor_node.GetSize() < node.GetSize())
        DebugPrint("BplusTreeHandler:got invalid nodes. neighbor node size %d, this node size %d\n",
                   neighbor_node.GetSize(), node.GetSize());
    if (index == 0) {
        // the neighbor is at right
        neighbor_node.MoveFirstToEnd(node, buffer_pool);
        parent_node.SetKeyAt(index + 1, neighbor_node.GetKeyAt(0));
    } else {
        // the neighbor is at left
        neighbor_node.MoveLastToFront(node, buffer_pool);
        parent_node.SetKeyAt(index, node.GetKeyAt(0));
    }
    neighbor_frame->DirtyMark();
    frame->DirtyMark();
    parent_frame->DirtyMark();
    buffer_pool->UnpinPage(parent_frame);
    buffer_pool->UnpinPage(neighbor_frame);
    buffer_pool->UnpinPage(frame);
    return Re::Success;
}
Re BplusTreeHandler::InsertEntryIntoParent(Frame *frame, Frame *new_frame, const char *key) {
    Re r;
    IndexNodeHandler node_handler(file_header, frame);
    IndexNodeHandler new_node_handler(file_header, new_frame);
    int32_t parent_page_id = node_handler.GetParentPageId();
    if (parent_page_id == BP_INVALID_PAGE_ID) {
        // create new root page
        Frame *root_frame = nullptr;
        r = buffer_pool->AllocatePage(&root_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to allocate new root page.r=%d:%s\n", r, StrRe(r));
            return r;
        }
        InternalIndexNodeHandler root_node(file_header, root_frame);
        root_node.InitEmpty();
        root_node.CreateNewRoot(frame->GetPageId(), key, new_frame->GetPageId());
        node_handler.SetParentPageId(root_frame->GetPageId());
        new_node_handler.SetParentPageId(root_frame->GetPageId());
        frame->DirtyMark();
        new_frame->DirtyMark();
        buffer_pool->UnpinPage(frame);
        buffer_pool->UnpinPage(new_frame);
        file_header.root_page_id = root_frame->GetPageId();
        UpdateRootPageId();
        root_frame->DirtyMark();
        buffer_pool->UnpinPage(root_frame);
        return Re::Success;
    } else {
        Frame *parent_frame = nullptr;
        r = buffer_pool->GetPage(parent_page_id, &parent_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to insert entry into leaf.r=%d:%s\n", r, StrRe(r));
            // should do more things to recover
            return r;
        }
        InternalIndexNodeHandler node(file_header, parent_frame);
        /// current node is not in full mode, insert the entry and return
        if (node.GetSize() < node.GetMaxSize()) {
            node.Insert(key, new_frame->GetPageId(), key_comparator);
            new_node_handler.SetParentPageId(parent_page_id);
            frame->DirtyMark();
            new_frame->DirtyMark();
            parent_frame->DirtyMark();
            buffer_pool->UnpinPage(frame);
            buffer_pool->UnpinPage(new_frame);
            buffer_pool->UnpinPage(parent_frame);
        } else {
            // we should split the node and insert the entry and then insert new entry to current node's parent
            Frame *new_parent_frame = nullptr;
            r = Split<InternalIndexNodeHandler>(parent_frame, new_parent_frame);
            if (r != Re::Success) {
                DebugPrint("BplusTreeHandler:failed to split internal node. r=%d:%s\n", r, StrRe(r));
                buffer_pool->UnpinPage(frame);
                buffer_pool->UnpinPage(new_frame);
                buffer_pool->UnpinPage(parent_frame);
            } else {
                // insert into left or right ? decide by key compare result
                InternalIndexNodeHandler new_node(file_header, new_parent_frame);
                if (key_comparator(key, new_node.GetKeyAt(0)) > 0) {
                    new_node.Insert(key, new_frame->GetPageId(), key_comparator);
                    new_node_handler.SetParentPageId(new_node.GetPageId());
                } else {
                    node.Insert(key, new_frame->GetPageId(), key_comparator);
                    new_node_handler.SetParentPageId(node.GetPageId());
                }
                buffer_pool->UnpinPage(frame);
                buffer_pool->UnpinPage(new_frame);
                r = InsertEntryIntoParent(parent_frame, new_parent_frame, new_node.GetKeyAt(0));
            }
        }
    }
    return Re::Success;
}
Re BplusTreeHandler::InsertEntryIntoLeafNode(Frame *frame, const char *key, const RecordId *rid) {
    LeafIndexNodeHandler leaf_node(file_header, frame);
    bool exists = false;
    int insert_position = leaf_node.LookUp(key_comparator, key, &exists);
    if (exists) {
        DebugPrint("BplusTreeHandler:entry exists\n");
        return Re::RecordDuplicateKey;
    }
    if (leaf_node.GetSize() < leaf_node.GetMaxSize()) {
        leaf_node.Insert(insert_position, key, (const char *) rid);
        frame->DirtyMark();
        buffer_pool->UnpinPage(frame);
        return Re::Success;
    }
    Frame *new_frame = nullptr;
    Re r = Split<LeafIndexNodeHandler>(frame, new_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to split leaf node.r=%d:%s\n", r, StrRe(r));
        return r;
    }
    LeafIndexNodeHandler new_index_node(file_header, new_frame);
    new_index_node.SetPrevPage(frame->GetPageId());
    new_index_node.SetNextPage(leaf_node.GetNextPage());
    new_index_node.SetParentPageId(leaf_node.GetParentPageId());
    leaf_node.SetNextPage(new_frame->GetPageId());
    int32_t next_page_id = new_index_node.GetNextPage();
    if (next_page_id != BP_INVALID_PAGE_ID) {
        Frame *next_frame = nullptr;
        r = buffer_pool->GetPage(next_page_id, &next_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to fetch next page. page id=%d, r=%d:%s\n", next_page_id, r, StrRe(r));
            return r;
        }
        LeafIndexNodeHandler next_node(file_header, next_frame);
        next_node.SetPrevPage(new_frame->GetPageId());
        buffer_pool->UnpinPage(next_frame);
    }
    if (insert_position < leaf_node.GetSize())
        leaf_node.Insert(insert_position, key, (const char *) rid);
    else
        new_index_node.Insert(insert_position - leaf_node.GetSize(), key, (const char *) rid);
    return InsertEntryIntoParent(frame, new_frame, new_index_node.GetKeyAt(0));
}
Re BplusTreeHandler::UpdateRootPageId() {
    Frame *header_frame = nullptr;
    Re r = buffer_pool->GetPage(FIRST_INDEX_PAGE, &header_frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch header page. r=%d:%s\n", r, StrRe(r));
        return r;
    }
    IndexFileHeader *header = reinterpret_cast<IndexFileHeader *>(header_frame->GetPageData());
    header->root_page_id = file_header.root_page_id;
    header_frame->DirtyMark();
    buffer_pool->UnpinPage(header_frame);
    return r;
}
Re BplusTreeHandler::CreateNewTree(const char *key, const RecordId *rid) {
    Re r;
    if (file_header.root_page_id != BP_INVALID_PAGE_ID) {
        r = Re::Internal;
        DebugPrint("BplusTreeHandler:cannot create new tree while root page is valid. root page id=%d\n",
                   file_header.root_page_id);
        return r;
    }
    Frame *frame = nullptr;
    r = buffer_pool->AllocatePage(&frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to allocate root page.r=%d:%s\n", r, StrRe(r));
        return r;
    }
    LeafIndexNodeHandler leaf_node(file_header, frame);
    leaf_node.InitEmpty();
    leaf_node.Insert(0, key, (const char *) rid);
    file_header.root_page_id = frame->GetPageId();
    frame->DirtyMark();
    buffer_pool->UnpinPage(frame);
    r = UpdateRootPageId();
    return Re::Success;
}
Re BplusTreeHandler::AdjustRoot(Frame *root_frame) {
    IndexNodeHandler root_node(file_header, root_frame);
    if (root_node.IsLeaf() and root_node.GetSize() > 0) {
        root_frame->DirtyMark();
        buffer_pool->UnpinPage(root_frame);
        return Re::Success;
    }
    if (root_node.IsLeaf())
        file_header.root_page_id = BP_INVALID_PAGE_ID;// this is a leaf and an empty node
    else {
        // this is an internal node and has only one child node
        InternalIndexNodeHandler internal_node(file_header, root_frame);
        const int32_t child_page_id = internal_node.GetValueAt(0);
        Frame *child_frame = nullptr;
        Re r = buffer_pool->GetPage(child_page_id, &child_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to fetch child page.page id=%d, r=%d:%s\n", child_page_id, r, StrRe(r));
            return r;
        }
        IndexNodeHandler child_node(file_header, child_frame);
        child_node.SetParentPageId(BP_INVALID_PAGE_ID);
        buffer_pool->UnpinPage(child_frame);
        file_header.root_page_id = child_page_id;
    }
    UpdateRootPageId();
    int32_t old_root_page_id = root_frame->GetPageId();
    buffer_pool->UnpinPage(root_frame);
    buffer_pool->DisposePage(old_root_page_id);
    return Re::Success;
}
Re BplusTreeHandler::PrintLeaf(Frame *frame) {
    LeafIndexNodeHandler leaf_node(file_header, frame);
    DebugPrint("BplusTreeHandler:leaf node: %s\n", ToString(leaf_node, key_printer).c_str());
    buffer_pool->UnpinPage(frame);
    return Re::Success;
}
Re BplusTreeHandler::PrintInternalNodeRecursive(Frame *frame) {
    Re r;
    DebugPrint("BplusTreeHandler:bplus tree. file header: %s\n", file_header.ToString().c_str());
    InternalIndexNodeHandler internal_node(file_header, frame);
    DebugPrint("BplusTreeHandler:internal node: %s", ToString(internal_node, key_printer).c_str());
    int node_size = internal_node.GetSize();
    for (int i = 0; i < node_size; i++) {
        int32_t page_id = internal_node.GetValueAt(i);
        Frame *child_frame = nullptr;
        r = buffer_pool->GetPage(page_id, &child_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to fetch child page.page id=%d,r=%d:%s\n", page_id, r, StrRe(r));
            buffer_pool->UnpinPage(frame);
            return r;
        }
        IndexNodeHandler node(file_header, child_frame);
        if (node.IsLeaf())
            r = PrintLeaf(child_frame);
        else
            r = PrintInternalNodeRecursive(child_frame);
        if (r != Re::Success) {
            DebugPrint("BplusTreeHandler:failed to print node.page id=%d,r=%d:%s\n", child_frame->GetPageId(), r,
                       StrRe(r));
            buffer_pool->UnpinPage(frame);
            return r;
        }
    }
    buffer_pool->UnpinPage(frame);
    return Re::Success;
}
bool BplusTreeHandler::IsValidateLeafLink() {
    if (IsEmpty())
        return true;
    Frame *frame = nullptr;
    Re r = LeftMostPage(frame);
    if (r != Re::Success) {
        DebugPrint("BplusTreeHandler:failed to fetch left most page.r=%d:%s\n", r, StrRe(r));
        return false;
    }
    int32_t prev_page_id = BP_INVALID_PAGE_ID;
    LeafIndexNodeHandler leaf_node(file_header, frame);
    if (leaf_node.GetPrevPage() != prev_page_id) {
        DebugPrint("BplusTreeHandler:invalid page. current_page_id=%d,prev page id should be %d but got %d\n",
                   frame->GetPageId(), prev_page_id, leaf_node.GetPrevPage());
        return false;
    }
    int32_t next_page_id = leaf_node.GetNextPage();
    prev_page_id = frame->GetPageId();
    char *prev_key = reinterpret_cast<char *>(mem_pool_item->Alloc());
    memmove(prev_key, leaf_node.GetKeyAt(leaf_node.GetSize() - 1), file_header.key_length);
    buffer_pool->UnpinPage(frame);
    bool result = true;
    while (result and next_page_id != BP_INVALID_PAGE_ID) {
        r = buffer_pool->GetPage(next_page_id, &frame);
        if (r != Re::Success) {
            FreeKey(prev_key);
            DebugPrint("BplusTreeHandler:failed to fetch next page.page id=%d,r=%d:%s\n", next_page_id, r, StrRe(r));
            return false;
        }
        LeafIndexNodeHandler leaf_node(file_header, frame);
        if (leaf_node.GetPrevPage() != prev_page_id) {
            DebugPrint("BplusTreeHandler:invalid page. current_page_id=%d, prev page id should be %d but got %d\n",
                       frame->GetPageId(), prev_page_id, leaf_node.GetPrevPage());
            result = false;
        }
        if (key_comparator(prev_key, leaf_node.GetKeyAt(0)) >= 0) {
            DebugPrint("BplusTreeHandler:invalid page. current first key is not bigger than last\n");
            result = false;
        }
        next_page_id = leaf_node.GetNextPage();
        memmove(prev_key, leaf_node.GetKeyAt(leaf_node.GetSize() - 1), file_header.key_length);
        prev_page_id = frame->GetPageId();
        buffer_pool->UnpinPage(frame);
    }
    FreeKey(prev_key);
    // can do more things
    return result;
}
bool BplusTreeHandler::IsValidateNodeRecursive(Frame *frame) {
    bool result = true;
    IndexNodeHandler node(file_header, frame);
    if (node.IsLeaf()) {
        LeafIndexNodeHandler leaf_node(file_header, frame);
        result = leaf_node.IsValidate(key_comparator, buffer_pool);
    } else {
        InternalIndexNodeHandler internal_node(file_header, frame);
        result = internal_node.IsValidate(key_comparator, buffer_pool);
        for (int i = 0; result and i < internal_node.GetSize(); i++) {
            int32_t page_id = internal_node.GetValueAt(i);
            Frame *child_frame = nullptr;
            Re r = buffer_pool->GetPage(page_id, &child_frame);
            if (r != Re::Success) {
                DebugPrint("BplusTreeHandler:failed to fetch child page.page id=%d, r=%d:%s\n", page_id, r, StrRe(r));
                result = false;
                break;
            }
            result = IsValidateNodeRecursive(child_frame);
        }
    }
    buffer_pool->UnpinPage(frame);
    return result;
}
/// @brief return a key and its struct is [field(data),record id];
/// @param user_key corresponding field of the index of record's data 
/// @param rid record id of the record 
char *BplusTreeHandler::MakeKey(const char *user_key, const RecordId &rid) {
    char *key = reinterpret_cast<char *>(mem_pool_item->Alloc());
    if (key == nullptr) {
        DebugPrint("BplusTreeHandler:failed to alloc memory for key.\n");
        return nullptr;
    }
    memmove(key, user_key, file_header.attr_length);
    memmove(key + file_header.attr_length, &rid, sizeof(rid));
    return key;
}
void BplusTreeHandler::FreeKey(char *key) {
    mem_pool_item->Free(key);
}
BplusTreeScanner::BplusTreeScanner(BplusTreeHandler &tree_handler)
    : inited_(false), left_frame_(nullptr), right_frame_(nullptr), iter_index_(-1), end_index_(-1),
      tree_handler_(tree_handler) {
}
BplusTreeScanner::~BplusTreeScanner() {
    Close();
}
Re BplusTreeScanner::Open(const char *left_user_key, int left_len, bool left_inclusive, const char *right_user_key,
                          int right_len, bool right_inclusive) {
    Re r;
    if (inited_) {
        DebugPrint("BplusTreeScanner:tree scanner has been inited\n");
        return Re::Internal;
    }
    inited_ = true;
    // 校验输入的键值是否是合法范围
    if (left_user_key and right_user_key) {
        const auto &attr_comparator = tree_handler_.key_comparator.GetAttrComparator();
        const int result = attr_comparator(left_user_key, right_user_key);
        if (result > 0 or (result == 0 and (!left_inclusive or !right_inclusive)))
            return Re::InvalidArgument;
    }
    if (left_user_key == nullptr) {
        r = tree_handler_.LeftMostPage(left_frame_);
        if (r != Re::Success) {
            DebugPrint("BplusTreeScanner:failed to find left most page. r=%d:%s\n", r, StrRe(r));
            return r;
        }
        iter_index_ = 0;
    } else {
        char *left_key = nullptr;
        char *fixed_left_key = const_cast<char *>(left_user_key);
        if (tree_handler_.file_header.attr_type == AttrType::Chars) {
            bool should_inclusive_after_fix = false;
            r = FixUserKey(left_user_key, left_len, true /*greater*/, &fixed_left_key, &should_inclusive_after_fix);
            if (r != Re::Success) {
                DebugPrint("BplusTreeScanner:failed to fix left user key. r=%s\n", StrRe(r));
                return r;
            }
            if (should_inclusive_after_fix)
                left_inclusive = true;
        }
        if (left_inclusive)
            left_key = tree_handler_.MakeKey(fixed_left_key, *RecordId::Min());
        else
            left_key = tree_handler_.MakeKey(fixed_left_key, *RecordId::Max());
        if (fixed_left_key != left_user_key) {
            delete[] fixed_left_key;
            fixed_left_key = nullptr;
        }
        r = tree_handler_.FindLeaf(left_key, left_frame_);
        if (r != Re::Success) {
            DebugPrint("BplusTreeScanner:failed to find left page. r=%d:%s\n", r, StrRe(r));
            tree_handler_.FreeKey(left_key);
            return r;
        }
        LeafIndexNodeHandler left_node(tree_handler_.file_header, left_frame_);
        int left_index = left_node.LookUp(tree_handler_.key_comparator, left_key);
        tree_handler_.FreeKey(left_key);
        // lookup 返回的是适合插入的位置，还需要判断一下是否在合适的边界范围内
        if (left_index >= left_node.GetSize()) {// 超出了当前页，就需要向后移动一个位置
            const int32_t next_page_id = left_node.GetNextPage();
            if (next_page_id == BP_INVALID_PAGE_ID)// 这里已经是最后一页，说明当前扫描，没有数据
                return Re::Success;
            tree_handler_.buffer_pool->UnpinPage(left_frame_);
            r = tree_handler_.buffer_pool->GetPage(next_page_id, &left_frame_);
            if (r != Re::Success) {
                DebugPrint("BplusTreeScanner:failed to fetch next page. page id=%d, r=%d:%s\n", next_page_id, r,
                           StrRe(r));
                return r;
            }
            left_index = 0;
        }
        iter_index_ = left_index;
    }
    // 没有指定右边界范围，那么就返回右边界最大值
    if (nullptr == right_user_key) {
        r = tree_handler_.RightMostPage(right_frame_);
        if (r != Re::Success) {
            DebugPrint("BplusTreeScanner:failed to fetch right most page. r=%d:%s\n", r, StrRe(r));
            return r;
        }
        LeafIndexNodeHandler node(tree_handler_.file_header, right_frame_);
        end_index_ = node.GetSize() - 1;
    } else {
        char *right_key = nullptr;
        char *fixed_right_key = const_cast<char *>(right_user_key);
        bool should_include_after_fix = false;
        if (tree_handler_.file_header.attr_type == AttrType::Chars) {
            r = FixUserKey(right_user_key, right_len, false /*want_greater*/, &fixed_right_key,
                           &should_include_after_fix);
            if (r != Re::Success) {
                DebugPrint("BplusTreeScanner:failed to fix right user key. r=%s\n", StrRe(r));
                return r;
            }
            if (should_include_after_fix)
                right_inclusive = true;
        }
        if (right_inclusive)
            right_key = tree_handler_.MakeKey(fixed_right_key, *RecordId::Max());
        else
            right_key = tree_handler_.MakeKey(fixed_right_key, *RecordId::Min());
        if (fixed_right_key != right_user_key) {
            delete[] fixed_right_key;
            fixed_right_key = nullptr;
        }
        r = tree_handler_.FindLeaf(right_key, right_frame_);
        if (r != Re::Success) {
            DebugPrint("BplusTreeScanner:failed to find left page. r=%d:%s\n", r, StrRe(r));
            tree_handler_.FreeKey(right_key);
            return r;
        }
        LeafIndexNodeHandler right_node(tree_handler_.file_header, right_frame_);
        int right_index = right_node.LookUp(tree_handler_.key_comparator, right_key);
        tree_handler_.FreeKey(right_key);
        // lookup 返回的是适合插入的位置，需要根据实际情况做调整
        // 通常情况下需要找到上一个位置
        if (right_index > 0)
            right_index--;
        else {
            // 实际上，只有最左边的叶子节点查找时，lookup 才可能返回0
            // 其它的叶子节点都不可能返回0，所以这段逻辑其实是可以简化的
            const int32_t prev_page_id = right_node.GetPrevPage();
            if (prev_page_id == BP_INVALID_PAGE_ID) {
                end_index_ = -1;
                return Re::Success;
            }
            tree_handler_.buffer_pool->UnpinPage(right_frame_);
            r = tree_handler_.buffer_pool->GetPage(prev_page_id, &right_frame_);
            if (r != Re::Success) {
                DebugPrint("BplusTreeScanner:failed to fetch prev page id. page id=%d, r=%d:%s\n", prev_page_id, r,
                           StrRe(r));
                return r;
            }
            LeafIndexNodeHandler tmp_node(tree_handler_.file_header, right_frame_);
            right_index = tmp_node.GetSize() - 1;
        }
        end_index_ = right_index;
    }
    // 判断是否左边界比右边界要靠后
    // 两个边界最多会多一页
    // 查找不存在的元素，或者不存在的范围数据时，可能会存在这个问题
    if (left_frame_->GetPageId() == right_frame_->GetPageId() and iter_index_ > end_index_)
        end_index_ = -1;
    else {
        LeafIndexNodeHandler left_node(tree_handler_.file_header, left_frame_);
        LeafIndexNodeHandler right_node(tree_handler_.file_header, right_frame_);
        if (left_node.GetPrevPage() == right_node.GetPageId())
            end_index_ = -1;
    }
    return Re::Success;
}
Re BplusTreeScanner::NextEntry(RecordId *rid) {
    if (end_index_ == -1)
        return Re::RecordEof;
    LeafIndexNodeHandler node(tree_handler_.file_header, left_frame_);
    memmove(rid, node.GetValueAt(iter_index_), sizeof(*rid));
    if (left_frame_->GetPageId() == right_frame_->GetPageId() and iter_index_ == end_index_) {
        end_index_ = -1;
        return Re::Success;
    }
    if (iter_index_ < node.GetSize() - 1) {
        ++iter_index_;
        return Re::Success;
    }
    Re r;
    if (left_frame_->GetPageId() != right_frame_->GetPageId()) {
        int32_t page_id = node.GetNextPage();
        tree_handler_.buffer_pool->UnpinPage(left_frame_);
        if (page_id == BP_INVALID_PAGE_ID) {
            left_frame_ = nullptr;
            DebugPrint("BplusTreeScanner:got invalid next page. page id=%d\n", page_id);
            r = Re::Internal;
        } else {
            r = tree_handler_.buffer_pool->GetPage(page_id, &left_frame_);
            if (r != Re::Success) {
                left_frame_ = nullptr;
                DebugPrint("BplusTreeScanner:failed to fetch next page. page id=%d, r=%d:%s\n", page_id, r, StrRe(r));
                return r;
            }
            iter_index_ = 0;
        }
    } else if (end_index_ != -1) {
        DebugPrint("BplusTreeScanner:should have more pages but not. left page=%d, right page=%d\n",
                   left_frame_->GetPageId(), right_frame_->GetPageId());
        r = Re::Internal;
    }
    return r;
}
Re BplusTreeScanner::Close() {
    if (left_frame_ != nullptr) {
        tree_handler_.buffer_pool->UnpinPage(left_frame_);
        left_frame_ = nullptr;
    }
    if (right_frame_ != nullptr) {
        tree_handler_.buffer_pool->UnpinPage(right_frame_);
        right_frame_ = nullptr;
    }
    end_index_ = -1;
    inited_ = false;
    DebugPrint("BplusTreeScanner:bplus tree scanner closed\n");
    return Re::Success;
}
Re BplusTreeScanner::FixUserKey(const char *user_key, int key_len, bool want_greater, char **fixed_key,
                                bool *should_inclusive) {
    if (fixed_key == nullptr or should_inclusive == nullptr)
        return Re::InvalidArgument;
    //变长字段才需要做调整，其它默认都不需要做调整
    assert(tree_handler_.file_header.attr_type == AttrType::Chars);
    assert(strlen(user_key) >= static_cast<size_t>(key_len));
    *should_inclusive = false;
    int32_t attr_length = tree_handler_.file_header.attr_length;
    char *key_buf = new (std::nothrow) char[attr_length];
    if (key_buf == nullptr)
        return Re::NoMem;
    if (key_len <= attr_length) {
        memmove(key_buf, user_key, key_len);
        memset(key_buf + key_len, 0, attr_length - key_len);
        *fixed_key = key_buf;
        return Re::Success;
    }
    // key_len > attr_length
    memmove(key_buf, user_key, attr_length);
    char c = user_key[attr_length];
    if (c == 0) {
        *fixed_key = key_buf;
        return Re::Success;
    }
    // 扫描 >=/> user_key 的数据
    // 示例：>=/> ABCD1 的数据，attr_length=4,
    //      等价于扫描 >= ABCE 的数据
    // 如果是扫描 <=/< user_key的数据
    // 示例：<=/< ABCD1  <==> <= ABCD  (attr_length=4)
    *should_inclusive = true;
    if (want_greater)
        key_buf[attr_length - 1]++;
    *fixed_key = key_buf;
    return Re::Success;
}