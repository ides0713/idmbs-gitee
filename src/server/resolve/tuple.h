#pragma once
#include "../storage/field.h"
#include "../storage/record.h"
#include "../storage/table.h"
enum class ExprType;
class Expression;
class FieldExpression;
class ValueExpression;
class TupleUnit
{
public:
    TupleUnit() : attr_type_(AttrType::Undefined), length_(-1), data_(nullptr) {}
    TupleUnit(AttrType type, char *data) : attr_type_(type), length_(-1), data_(data) {}
    TupleUnit(FieldMeta *meta, char *data) : TupleUnit(meta->GetAttrType(), data) {}
    void SetAttrType(AttrType type) { attr_type_ = type; }
    void SetLength(int length) { length_ = length; }
    void SetData(char *data) { data_ = data; }
    [[nodiscard]] AttrType GetAttrType() const { return attr_type_; }
    [[nodiscard]] int GetLength() const { return length_; }
    [[nodiscard]] const char *GetData() const { return data_; }
    void ToString(std::ostream &os) const;
    int Compare(const TupleUnit &other);

private:
    AttrType attr_type_;
    int length_;
    char *data_;
};
class TupleUnitSpec
{
public:
    TupleUnitSpec() : alias_(nullptr), expression_(nullptr) {}
    explicit TupleUnitSpec(Expression *expr) : expression_(expr) {}
    ~TupleUnitSpec();
    void SetAlias(const char *alias) { this->alias_ = alias; }
    void SetExpression(Expression *expr) { expression_ = expr; }
    [[nodiscard]] const char *GetAlias() const { return alias_; }
    [[nodiscard]] Expression *GetExpression() const { return expression_; }

private:
    const char *alias_;
    Expression *expression_;
};
class Tuple
{
public:
    Tuple() = default;
    virtual ~Tuple() = default;
    [[nodiscard]] virtual int GetUnitsNum() const = 0;
    virtual Re GetUnitAt(int index, TupleUnit &unit) const = 0;
    virtual Re GetUnit(const Field &field, TupleUnit &unit) const = 0;
    virtual Re GetUnitSpecAt(int index, const TupleUnitSpec *&spec) const = 0;
};
class RowTuple : public Tuple
{
public:
    RowTuple() : record_(nullptr), table_(nullptr) {}
    ~RowTuple() override;
    void SetRecord(class Record *record) { record_ = record; }
    void SetSchema(const Table *table, const std::vector<FieldMeta> *fields);
    [[nodiscard]] int GetUnitsNum() const override { return specs_.size(); }
    Re GetUnitAt(int index, TupleUnit &unit) const override;
    Re GetUnit(const Field &field, TupleUnit &unit) const override;
    Re GetUnitSpecAt(int index, const TupleUnitSpec *&spec) const override;
    class Record &GetRecord()
    {
        return *record_;
    }
    [[nodiscard]] const class Record &GetRecord() const { return *record_; }
    const Table &GetTable() { return *table_; }

private:
    class Record *record_;
    const Table *table_;
    std::vector<TupleUnitSpec *> specs_;
};
class ProjectTuple : public Tuple
{
public:
    ProjectTuple() : tuple_(nullptr) {}
    ~ProjectTuple() override;
    void SetTuple(Tuple *tuple) { tuple_ = tuple; }
    void AddUnitSpec(TupleUnitSpec *spec) { specs_.push_back(spec); }
    [[nodiscard]] int GetUnitsNum() const override { return specs_.size(); }
    Re GetUnitAt(int index, TupleUnit &unit) const override;
    Re GetUnit(const Field &field, TupleUnit &unit) const override;
    Re GetUnitSpecAt(int index, const TupleUnitSpec *&unit) const override;
    Tuple *GetTuple() { return tuple_; }

private:
    std::vector<TupleUnitSpec *> specs_;
    Tuple *tuple_;
};