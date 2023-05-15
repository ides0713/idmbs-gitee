#pragma once
#include "b_plus_tree.h"
#include "index.h"
class BplusTreeIndex : public Index
{
public:
    BplusTreeIndex() : inited_(false){};
    virtual ~BplusTreeIndex() noexcept;
    Re Create(const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta);
    Re Open(const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta);
    Re Close();
    Re InsertEntry(const char *record, const RecordId *rid) override;
    Re DeleteEntry(const char *record, const RecordId *rid) override;
    /**
   * 扫描指定范围的数据
   */
    IndexScanner *CreateScanner(const char *left_key, int left_len, bool left_inclusive, const char *right_key,
                                int right_len, bool right_inclusive) override;
    Re Sync() override;

private:
    bool inited_;
    BplusTreeHandler index_handler_;
};
class BplusTreeIndexScanner : public IndexScanner
{
public:
    BplusTreeIndexScanner(BplusTreeHandler &tree_handle);
    ~BplusTreeIndexScanner() noexcept override;
    Re NextEntry(RecordId *rid) override;
    Re Destroy() override;
    Re Open(const char *left_key, int left_len, bool left_inclusive, const char *right_key, int right_len,
            bool right_inclusive);

private:
    BplusTreeScanner tree_scanner_;
};