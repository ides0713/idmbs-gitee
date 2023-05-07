#pragma once
#include "../../common/common_defs.h"
#include "../parse/parse_defs.h"
// #include "buffer_pool.h"
#include <functional>
#include <sstream>
#include <string.h>
#define EMPTY_RID_PAGE_ID -1
#define EMPTY_RID_SLOT_ID -1
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