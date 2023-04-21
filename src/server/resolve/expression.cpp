#include "expression.h"

Re FieldExpression::getValue(const Tuple &tuple, TupleUnit &unit) const
{
    return tuple.getUnit(field_, unit);
}

ValueExpression::ValueExpression(const Value &value) : tuple_unit_(value.type, reinterpret_cast<char *>(value.data))
{
    if (value.type == AttrType::Chars)
        tuple_unit_.setLength(strlen(reinterpret_cast<const char *>(value.data)));
}

Re ValueExpression::getValue(const Tuple &tuple, TupleUnit &unit) const
{
    unit = tuple_unit_;
    return Re::Success;
}

std::string strExprType(ExprType type)
{
    switch (type)
    {
    case ExprType::Field:
        return std::string{"FieldExpression"};
    case ExprType::Value:
        return std::string{"ValueExpression"};
    case ExprType::None:
    default:
        assert(false);
    }
}
