#pragma once

#include "../storage/field.h"
#include "tuple.h"
#include <cstring>
#include <string>

enum class ExprType {
    None,
    Field,
    Value
};

std::string StrExprType(ExprType type);
class Expression {
public:
    Expression() = default;

    virtual ~Expression() = default;

    virtual Re GetValue(const Tuple &tuple, TupleUnit &unit) const = 0;

    virtual ExprType GetExprType() const = 0;
};

class FieldExpression : public Expression {
public:
    FieldExpression() = default;

    FieldExpression(const Table *table, const FieldMeta *field) : field_(table, field) {}

    ~FieldExpression() override = default;

    [[nodiscard]] ExprType GetExprType() const override { return ExprType::Field; }

    Field &GetField() { return field_; }

    [[nodiscard]] const Field &GetField() const { return field_; }

    [[nodiscard]] const char *GetTableName() const { return field_.GetTableName(); }

    [[nodiscard]] const char *GetFieldName() const { return field_.GetFieldName(); }

    Re GetValue(const Tuple &tuple, TupleUnit &unit) const override;

private:
    Field field_;
};

class ValueExpression : public Expression {
public:
    ValueExpression() = default;

    explicit ValueExpression(const Value &value);

    ~ValueExpression() override = default;

    [[nodiscard]] ExprType GetExprType() const override { return ExprType::Value; }

    void GetTupleUnit(TupleUnit &unit) const { unit = tuple_unit_; }

    Re GetValue(const Tuple &tuple, TupleUnit &unit) const override;

private:
    TupleUnit tuple_unit_;
};