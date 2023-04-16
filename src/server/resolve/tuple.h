#pragma once

#include "../storage/table.h"
#include "../storage/field.h"
#include "../storage/record.h"

enum class ExprType;

class Expression;

class FieldExpression;

class ValueExpression;

class TupleUnit {
public:
    TupleUnit() : attr_type_(AttrType::Undefined), length_(-1), data_(nullptr) {}

    TupleUnit(AttrType type, char *data) : attr_type_(type), length_(-1), data_(data) {}

    TupleUnit(FieldMeta *meta, char *data) : TupleUnit(meta->getAttrType(), data) {}

    void setAttrType(AttrType type) { attr_type_ = type; }

    void setLength(int length) { length_ = length; }

    void setData(char *data) { data_ = data; }

    [[nodiscard]] AttrType getAttrType() const { return attr_type_; }

    [[nodiscard]] int getLength() const { return length_; }

    [[nodiscard]] const char *getData() const { return data_; }

    void toString(std::ostream &os) const;

private:
    AttrType attr_type_;
    int length_;
    char *data_;
};

class TupleUnitSpec {
public:
    TupleUnitSpec() : alias_(nullptr), expression_(nullptr) {}

    explicit TupleUnitSpec(Expression *expr) : expression_(expr) {}

    ~TupleUnitSpec();

    void setAlias(const char *alias) { this->alias_ = alias; }

    void setExpression(Expression *expr) { expression_ = expr; }

    [[nodiscard]] const char *getAlias() const { return alias_; }

    [[nodiscard]] Expression *getExpression() const { return expression_; }

private:
    const char *alias_;
    Expression *expression_;
};

class Tuple {
public:
    Tuple() = default;

    virtual ~Tuple() = default;

    [[nodiscard]] virtual int getUnitsNum() const = 0;

    virtual Re getUnitAt(int index, TupleUnit &unit) const = 0;

    virtual Re getUnit(const Field &field, TupleUnit &unit) const = 0;

    virtual Re getUnitSpecAt(int index, const TupleUnitSpec *&spec) const = 0;
};

class RowTuple : public Tuple {
public:
    RowTuple() : record_(nullptr), table_(nullptr) {}

    ~RowTuple() override;

    void setRecord(class Record *record) { record_ = record; }

    void setSchema(const Table *table, const std::vector<FieldMeta> *fields);

    [[nodiscard]] int getUnitsNum() const override { return specs_.size(); }

    Re getUnitAt(int index, TupleUnit &unit) const override;

    Re getUnit(const Field &field, TupleUnit &unit) const override;

    Re getUnitSpecAt(int index, const TupleUnitSpec *&spec) const override;

    class Record &record() { return *record_; }

    [[nodiscard]] const class Record &record() const { return *record_; }

private:
    class Record *record_;

    const Table *table_;
    std::vector<TupleUnitSpec *> specs_;
};

class ProjectTuple : public Tuple {
public:
    ProjectTuple() : tuple_(nullptr) {}

    ~ProjectTuple() override;

    void setTuple(Tuple *tuple) { tuple_ = tuple; }

    void addUnitSpec(TupleUnitSpec *spec) { specs_.push_back(spec); }

    [[nodiscard]] int getUnitsNum() const override { return specs_.size(); }

    Re getUnitAt(int index, TupleUnit &unit) const override;

    Re getUnit(const Field &field, TupleUnit &unit) const override;

    Re getUnitSpecAt(int index, const TupleUnitSpec *&unit) const override;

private:
    std::vector<TupleUnitSpec *> specs_;
    Tuple *tuple_;
};