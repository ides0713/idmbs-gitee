#include "b_plus_tree.h"
#include "buffer_pool.h"
#include "record.h"
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
    root_page_id = BP_INVALID_PAGE_NUM;
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
IndexNodeHandler::IndexNodeHandler(const IndexFileHeader &header, Frame *frame)
    : header_(header), page_id_(frame->GetPageId()), node_(reinterpret_cast<IndexNode *>(frame->GetPageData())) {
}
void IndexNodeHandler::InitEmpty(bool is_leaf) {
    node_->is_leaf = is_leaf;
    node_->key_num = 0;
    node_->parent = BP_INVALID_PAGE_NUM;
}
bool IndexNodeHandler::IsLeaf() const {
    return node_->is_leaf;
}
int IndexNodeHandler::GetKeySize() const {
    return header_.key_length;
}
int IndexNodeHandler::GetValueSize() const {
    return sizeof(RecordId);
}
int IndexNodeHandler::GetItemSize() const {
    return GetKeySize() + GetValueSize();
}
void IndexNodeHandler::IncreaseSize(int n) {
    node_->key_num += n;
}
int IndexNodeHandler::GetSize() const {
    return node_->key_num;
}
void IndexNodeHandler::SetParentPageId(int32_t page_id) {
    node_->parent = page_id;
}
int32_t IndexNodeHandler::GetParentPageId() const {
    return node_->parent;
}
int32_t IndexNodeHandler::GetPageId() const {
    return page_id_;
}
bool IndexNodeHandler::IsValidate() const {
    if (GetParentPageId() == BP_INVALID_PAGE_NUM) {
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
