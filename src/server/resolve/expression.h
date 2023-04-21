#pragma once

#include <cstring>
#include <string>
#include "../storage/field.h"
#include "tuple.h"

enum class ExprType
{
    None,
    Field,
    Value
};

std::string strExprType(ExprType type);
class Expression
{
public:
    Expression() = default;

    virtual ~Expression() = default;

    virtual Re getValue(const Tuple &tuple, TupleUnit &unit) const = 0;

    virtual ExprType getExprType() const = 0;
};

class FieldExpression : public Expression
{
public:
    FieldExpression() = default;

    FieldExpression(const Table *table, const FieldMeta *field) : field_(table, field) {}

    ~FieldExpression() override = default;

    [[nodiscard]] ExprType getExprType() const override { return ExprType::Field; }

    Field &getField() { return field_; }

    [[nodiscard]] const Field &getField() const { return field_; }

    [[nodiscard]] const char *getTableName() const { return field_.getTableName(); }

    [[nodiscard]] const char *getFieldName() const { return field_.getFieldName(); }

    Re getValue(const Tuple &tuple, TupleUnit &unit) const override;

private:
    Field field_;
};

class ValueExpression : public Expression
{
public:
    ValueExpression() = default;

    explicit ValueExpression(const Value &value);

    ~ValueExpression() override = default;

    [[nodiscard]] ExprType getExprType() const override { return ExprType::Value; }

    void getTupleUnit(TupleUnit &unit) const { unit = tuple_unit_; }

    Re getValue(const Tuple &tuple, TupleUnit &unit) const override;

private:
    TupleUnit tuple_unit_;
};