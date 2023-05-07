#include "b_plus_tree.h"
#include "record.h"
#include "buffer_pool.h"
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
