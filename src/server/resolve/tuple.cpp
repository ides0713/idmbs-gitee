#include "tuple.h"
#include "../../common/common_defs.h"
#include "expression.h"

std::string double2string(double v)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%.2f", v);
    size_t len = strlen(buf);
    while (buf[len - 1] == '0')
    {
        len--;
    }
    if (buf[len - 1] == '.')
    {
        len--;
    }

    return std::string(buf, len);
}

void TupleUnit::toString(std::ostream &os) const
{
    switch (attr_type_)
    {
    case AttrType::Ints:
        os << *reinterpret_cast<int *>(data_);
        break;
    case AttrType::Floats:
        os << double2string(*reinterpret_cast<float *>(data_));
        break;
    case AttrType::Chars:
        for (int i = 0; i < length_; i++)
        {
            if (data_[i] == '\0')
                break;
            os << data_[i];
        }
        break;
    case AttrType::Dates:
        //            int val = *(int *)data_;
        //            char buf[16] = {'\0'};
        //            snprintf(buf, sizeof(buf), "%04d-%02d-%02d", val / 10000, (val % 10000) / 100, val % 100);
        //            os << buf;
        assert(false);
        break;
    default:
        debugPrint("TupleUnit:unsupported attr getExprType: %d\n", attr_type_);
        break;
    }
}

int TupleUnit::compare(const TupleUnit &other)
{
    if (attr_type_ == other.attr_type_)
    {
        switch (attr_type_)
        {
        case AttrType::Ints:
        {
            int int_1 = *reinterpret_cast<int *>(data_), int_2 = *reinterpret_cast<int *>(other.data_);
            return int_1 - int_2;
        }
        case AttrType::Undefined:
            printf("undefined compare not implemented yet\n");
            assert(false);
        case AttrType::Chars:
        {
            const char *str_1 = reinterpret_cast<const char *>(data_), *str_2 = reinterpret_cast<const char *>(other.data_);
            int compare_len = std::min(length_, other.length_);
            int compare_result = strncmp(str_1, str_2, compare_len);
            if (compare_result != 0)
                return compare_result;
            if (length_ == other.length_)
                return compare_result;
            if (length_ > other.length_)
                return str_1[compare_len] - '\0';
            return str_2[compare_len] - '\0';
        }
        case AttrType::Floats:
        {
            float res = *reinterpret_cast<float *>(data_) - *reinterpret_cast<float *>(other.data_);
            if (res > 0)
                return 1;
            if (res < 0)
                return -1;
            return 0;
        }
        case AttrType::Dates:
            printf("dates compare not implemented yet\n");
            assert(false);
        }
    }
    assert(false);
}

TupleUnitSpec::~TupleUnitSpec()
{
    delete expression_;
    expression_ = nullptr;
}

RowTuple::~RowTuple()
{
    for (auto spec : specs_)
        delete spec;
    specs_.clear();
}

void RowTuple::setSchema(const Table *table, const std::vector<FieldMeta> *fields)
{
    table_ = table;
    specs_.reserve(fields->size());
    for (const FieldMeta &field : *fields)
        specs_.push_back(new TupleUnitSpec(new FieldExpression(table, &field)));
}

Re RowTuple::getUnitAt(int index, TupleUnit &unit) const
{
    if (index < 0 or index >= specs_.size())
    {
        debugPrint("RowTuple:invalid argument. index=%d\n", index);
        return Re::InvalidArgument;
    }
    const TupleUnitSpec *spec = specs_[index];
    auto field_expr = static_cast<FieldExpression *>(spec->getExpression());
    const FieldMeta *field_meta = field_expr->getField().getFieldMeta();
    unit.setAttrType(field_meta->getAttrType());
    unit.setData(record_->getData() + field_meta->getOffset());
    unit.setLength(field_meta->getLen());
    return Re::Success;
}

Re RowTuple::getUnit(const Field &field, TupleUnit &unit) const
{
    if (strcmp(table_->getTableName(), field.getTableName()) != 0)
        return Re::NotFound;
    for (int i = 0; i < specs_.size(); i++)
    {
        auto field_expr = static_cast<FieldExpression *>(specs_[i]->getExpression());
        const Field &temp_field = field_expr->getField();
        if (strcmp(field.getFieldName(), temp_field.getFieldName()) == 0)
            return getUnitAt(i, unit);
    }
    return Re::NotFound;
}

Re RowTuple::getUnitSpecAt(int index, const TupleUnitSpec *&spec) const
{
    if (index < 0 or index >= specs_.size())
    {
        debugPrint("RowTuple:invalid argument. index=%d\n", index);
        return Re::InvalidArgument;
    }
    spec = specs_[index];
    return Re::Success;
}

ProjectTuple::~ProjectTuple()
{
    for (auto spec : specs_)
        delete spec;
    specs_.clear();
}

Re ProjectTuple::getUnitAt(int index, TupleUnit &unit) const
{
    if (index < 0 or index >= specs_.size())
        return Re::GenericError;
    if (tuple_ == nullptr)
        return Re::GenericError;
    const TupleUnitSpec *spec = specs_[index];
    return spec->getExpression()->getValue(*tuple_, unit);
}

Re ProjectTuple::getUnit(const Field &field, TupleUnit &unit) const
{
    return tuple_->getUnit(field, unit);
}

Re ProjectTuple::getUnitSpecAt(int index, const TupleUnitSpec *&unit) const
{
    if (index < 0 or index >= specs_.size())
        return Re::NotFound;
    unit = specs_[index];
    return Re::Success;
}
