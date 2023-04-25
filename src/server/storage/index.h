#pragma once
#include <string>          // for string

#include "../common/re.h"  // for Re

class FieldMeta;
class TableMeta;

namespace Json {
    class Value;
}
class IndexMeta
{
public:
    IndexMeta() = default;
    Re Init(const char *name, const FieldMeta &field);
    [[nodiscard]] std::string GetIndexName() const;
    [[nodiscard]] std::string GetFieldName() const;
    //    void desc(std::ostream &os) const;
    void ToJson(Json::Value &json_value) const;

    const char * GetIndexName(){return index_name_.c_str();}
    const char * GetFieldName(){return field_name_.c_str();}
public:
    static Re FromJson(const TableMeta &table_meta, const Json::Value &json_value, IndexMeta &index_meta);

private:
    std::string index_name_, field_name_;
};

class IndexDataOperator {
public:
  virtual ~IndexDataOperator() = default;
  virtual int Compare(const void *data1, const void *data2) const = 0;
  virtual size_t Hash(const void *data) const = 0;
};

class IndexScanner;

class Index {

public:
  Index() = default;
  virtual ~Index() = default;

  const IndexMeta &index_meta() const
  {
    return index_meta_;
  }

  virtual Re insert_entry(const char *record, const RID *rid) = 0;
  virtual Re delete_entry(const char *record, const RID *rid) = 0;

  virtual IndexScanner *create_scanner(const char *left_key, int left_len, bool left_inclusive,
				       const char *right_key, int right_len, bool right_inclusive) = 0;

  virtual RC sync() = 0;

protected:
  RC init(const IndexMeta &index_meta, const FieldMeta &field_meta);

protected:
  IndexMeta index_meta_;
  FieldMeta field_meta_;  /// 当前实现仅考虑一个字段的索引
};

class IndexScanner {
public:
  IndexScanner() = default;
  virtual ~IndexScanner() = default;

  /**
   * 遍历元素数据
   * 如果没有更多的元素，返回RECORD_EOF
   */
  virtual RC next_entry(RID *rid) = 0;
  virtual RC destroy() = 0;
};