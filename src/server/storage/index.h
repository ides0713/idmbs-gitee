#pragma once
#include "index_meta.h"
#include "field.h"
#include "record.h"
class IndexDataOperator
{
public:
    virtual ~IndexDataOperator() = default;
    virtual int Compare(const void *data1, const void *data2) const = 0;
    virtual size_t Hash(const void *data) const = 0;
};
class IndexScanner;
//implementation now only support index on one field

class Index
{
public:
    Index() = default;
    virtual ~Index() = default;
    const IndexMeta &GetIndexMeta() const { return index_meta_; }
    virtual Re InsertEntry(const char *record, const RecordId *rid) = 0;
    virtual Re DeleteEntry(const char *record, const RecordId *rid) = 0;
    virtual IndexScanner *CreateScanner(const char *left_key, int left_len, bool left_inclusive, const char *right_key,
                                         int right_len, bool right_inclusive) = 0;
    virtual Re Sync() = 0;

protected:
    Re Init(const IndexMeta &index_meta, const FieldMeta &field_meta);

protected:
    IndexMeta index_meta_;
    FieldMeta field_meta_;
};
class IndexScanner
{
public:
    IndexScanner() = default;
    virtual ~IndexScanner() = default;
    virtual Re NextEntry(RecordId *rid) = 0;
    virtual Re Destroy() = 0;
};