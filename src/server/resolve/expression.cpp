#include "expression.h"

#include <assert.h>                                          // for assert
#include <cstring>                                           // for strlen

#include "../parse/parse_defs.h"  // for Value

Re FieldExpression::GetValue(const Tuple &tuple, TupleUnit &unit) const {
    return tuple.GetUnit(field_, unit);
}
ValueExpression::ValueExpression(const Value &value) : tuple_unit_(value.type, reinterpret_cast<char *>(value.data)) {
    if (value.type == AttrType::Chars)
        tuple_unit_.SetLength(strlen(reinterpret_cast<const char *>(value.data)));
}
Re ValueExpression::GetValue(const Tuple &tuple, TupleUnit &unit) const {
    unit = tuple_unit_;
    return Re::Success;
}
std::string StrExprType(ExprType type) {
    switch (type) {
        case ExprType::Field:
            return std::string{"FieldExpression"};
        case ExprType::Value:
            return std::string{"ValueExpression"};
        case ExprType::None:
        default:
            assert(false);
    }
}
