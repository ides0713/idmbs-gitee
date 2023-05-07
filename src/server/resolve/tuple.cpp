#include "tuple.h"
#include "../../common/common_defs.h"// for DebugPrint
#include "../storage/field.h"        // for Field, Fie...
#include "../storage/record.h"       // for Record
#include "../storage/table.h"        // for Table
#include "expression.h"              // for FieldExpre...
#include <algorithm>                 // for min
#include <assert.h>                  // for assert
#include <ostream>                   // for ostream
#include <stdio.h>                   // for printf
#include <string.h>                  // for strcmp
#include <string>                    // for operator<<
std::string Double2string(double v) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%.2f", v);
    size_t len = strlen(buf);
    while (buf[len - 1] == '0') {
        len--;
    }
    if (buf[len - 1] == '.') {
        len--;
    }
    return std::string(buf, len);
}
void TupleUnit::ToString(std::ostream &os) const {
    switch (attr_type_) {
        case AttrType::Ints:
            os << *reinterpret_cast<int *>(data_);
            break;
        case AttrType::Floats:
            os << Double2string(*reinterpret_cast<float *>(data_));
            break;
        case AttrType::Chars:
            for (int i = 0; i < length_; i++) {
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
            DebugPrint("TupleUnit:unsupported attr getExprType: %d\n", attr_type_);
            break;
    }
}
int TupleUnit::Compare(const TupleUnit &other) {
    if (attr_type_ == other.attr_type_) {
        switch (attr_type_) {
            case AttrType::Ints:
                return CompareInt((void *) data_, (void *) other.data_);
            case AttrType::Chars:
                return CompareString((void *) data_, length_, (void *) other.data_, other.length_);
            case AttrType::Floats:
                return CompareFloat((void *) data_, (void *) other.data_);
            case AttrType::Dates:
                printf("dates compare not implemented yet\n");
                assert(false);
            case AttrType::Undefined:
                printf("undefined compare not implemented yet\n");
                assert(false);
            default:
                assert(false);
        }
    }
}
TupleUnitSpec::~TupleUnitSpec() {
    delete expression_;
    expression_ = nullptr;
}
RowTuple::~RowTuple() {
    for (auto spec: specs_)
        delete spec;
    specs_.clear();
}
void RowTuple::SetSchema(const Table *table, const std::vector<FieldMeta> *fields) {
    table_ = table;
    specs_.reserve(fields->size());
    for (const FieldMeta &field: *fields)
        specs_.push_back(new TupleUnitSpec(new FieldExpression(table, &field)));
}
Re RowTuple::GetUnitAt(int index, TupleUnit &unit) const {
    if (index < 0 or index >= specs_.size()) {
        DebugPrint("RowTuple:invalid argument. index=%d\n", index);
        return Re::InvalidArgument;
    }
    const TupleUnitSpec *spec = specs_[index];
    auto field_expr = static_cast<FieldExpression *>(spec->GetExpression());
    const FieldMeta *field_meta = field_expr->GetField().GetFieldMeta();
    unit.SetAttrType(field_meta->GetAttrType());
    unit.SetData(record_->GetData() + field_meta->GetOffset());
    unit.SetLength(field_meta->GetLen());
    return Re::Success;
}
Re RowTuple::GetUnit(const Field &field, TupleUnit &unit) const {
    if (strcmp(table_->GetTableName(), field.GetTableName()) != 0)
        return Re::NotFound;
    for (int i = 0; i < specs_.size(); i++) {
        auto field_expr = static_cast<FieldExpression *>(specs_[i]->GetExpression());
        const Field &temp_field = field_expr->GetField();
        if (strcmp(field.GetFieldName(), temp_field.GetFieldName()) == 0)
            return GetUnitAt(i, unit);
    }
    return Re::NotFound;
}
Re RowTuple::GetUnitSpecAt(int index, const TupleUnitSpec *&spec) const {
    if (index < 0 or index >= specs_.size()) {
        DebugPrint("RowTuple:invalid argument. index=%d\n", index);
        return Re::InvalidArgument;
    }
    spec = specs_[index];
    return Re::Success;
}
ProjectTuple::~ProjectTuple() {
    for (auto spec: specs_)
        delete spec;
    specs_.clear();
}
Re ProjectTuple::GetUnitAt(int index, TupleUnit &unit) const {
    if (index < 0 or index >= specs_.size())
        return Re::GenericError;
    if (tuple_ == nullptr)
        return Re::GenericError;
    const TupleUnitSpec *spec = specs_[index];
    return spec->GetExpression()->GetValue(*tuple_, unit);
}
Re ProjectTuple::GetUnit(const Field &field, TupleUnit &unit) const {
    return tuple_->GetUnit(field, unit);
}
Re ProjectTuple::GetUnitSpecAt(int index, const TupleUnitSpec *&unit) const {
    if (index < 0 or index >= specs_.size())
        return Re::NotFound;
    unit = specs_[index];
    return Re::Success;
}
