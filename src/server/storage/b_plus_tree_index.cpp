#include "b_plus_tree_index.h"
#include "../common/persist_file_io_handler.h"
BplusTreeIndex::~BplusTreeIndex() noexcept {
    Close();
}
Re BplusTreeIndex::Create(const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) {
    if (inited_) {
        DebugPrint("BplusTreeIndex:failed to create index due to the index has been created before. file_name:%s, "
                   "index:%s, field:%s\n",
                   file_name, index_meta.GetIndexName(), index_meta.GetFieldName());
        return Re::RecordOpened;
    }
    Index::Init(index_meta, field_meta);
    Re r = index_handler_.Create(file_name, field_meta.GetAttrType(), field_meta.GetLen());
    if (Re::Success != r) {
        DebugPrint("BplusTreeIndex:failed to create index_handler, file_name:%s, index:%s, field:%s, r:%s\n", file_name,
                   index_meta.GetIndexName(), index_meta.GetFieldName(), StrRe(r));
        return r;
    }
    inited_ = true;
    DebugPrint("BplusTreeIndex:successfully create index, file_name:%s, index:%s, field:%s\n", file_name,
               index_meta.GetIndexName(), index_meta.GetFieldName());
    return Re::Success;
}
Re BplusTreeIndex::Open(const char *file_name, const IndexMeta &index_meta, const FieldMeta &field_meta) {
    if (inited_) {
        DebugPrint("BplusTreeIndex:Failed to open index due to the index has been initedd before. file_name:%s, "
                   "index:%s, field:%s",
                   file_name, index_meta.GetIndexName(), index_meta.GetFieldName());
        return Re::RecordOpened;
    }
    Index::Init(index_meta, field_meta);
    Re r = index_handler_.Open(file_name);
    if (Re::Success != r) {
        DebugPrint("BplusTreeIndex:failed to open index_handler, file_name:%s, index:%s, field:%s, r:%s\n", file_name,
                   index_meta.GetIndexName(), index_meta.GetFieldName(), StrRe(r));
        return r;
    }
    inited_ = true;
    DebugPrint("BplusTreeIndex:successfully open index, file_name:%s, index:%s, field:%s\n", file_name,
               index_meta.GetIndexName(), index_meta.GetFieldName());
    return Re::Success;
}
Re BplusTreeIndex::Close() {
    if (inited_) {
        DebugPrint("BplusTreeIndex:begin to close index, index:%s, field:%s\n", index_meta_.GetIndexName(),
                   index_meta_.GetFieldName());
        Re r=index_handler_.Close();
        if(r!=Re::Success){
            DebugPrint("BplusTreeIndex:close index failed,r=%d:%s\n",r,StrRe(r));
            return r;
        }
        inited_ = false;
    }
    DebugPrint("BplusTreeIndex:successfully close index.\n");
    return Re::Success;
}
Re BplusTreeIndex::InsertEntry(const char *record, const RecordId *rid) {
    return index_handler_.InsertEntry(record + field_meta_.GetOffset(), rid);
}
Re BplusTreeIndex::DeleteEntry(const char *record, const RecordId *rid) {
    return index_handler_.DeleteEntry(record + field_meta_.GetOffset(), rid);
}
IndexScanner *BplusTreeIndex::CreateScanner(const char *left_key, int left_len, bool left_inclusive,
                                            const char *right_key, int right_len, bool right_inclusive) {
    BplusTreeIndexScanner *index_scanner = new BplusTreeIndexScanner(index_handler_);
    Re r = index_scanner->Open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
    if (r != Re::Success) {
        DebugPrint("BplusTreeIndex:failed to open index scanner. r=%d:%s\n", r, StrRe(r));
        delete index_scanner;
        return nullptr;
    }
    return index_scanner;
}
Re BplusTreeIndex::Sync() {
    return index_handler_.Sync();
}
BplusTreeIndexScanner::BplusTreeIndexScanner(BplusTreeHandler &tree_handler) : tree_scanner_(tree_handler) {
}
BplusTreeIndexScanner::~BplusTreeIndexScanner() noexcept {
    tree_scanner_.Close();
}
Re BplusTreeIndexScanner::NextEntry(RecordId *rid) {
    return tree_scanner_.NextEntry(rid);
}
Re BplusTreeIndexScanner::Destroy() {
    delete this;
    return Re::Success;
}
Re BplusTreeIndexScanner::Open(const char *left_key, int left_len, bool left_inclusive, const char *right_key,
                               int right_len, bool right_inclusive) {
    return tree_scanner_.Open(left_key, left_len, left_inclusive, right_key, right_len, right_inclusive);
}
