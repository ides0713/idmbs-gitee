#include "condition_filter.h"
#include "../../common/common_defs.h"
#include "cmath"
#include "table.h"
void ConDesc::Init(bool is_attr, int attr_length, int attr_offset, void *value) {
    this->is_attr = is_attr;
    this->attr_length = attr_length;
    this->attr_offset = attr_offset;
    this->value = value;
}
DefaultConditionFilter::DefaultConditionFilter() : attr_type_(AttrType::Undefined), comp_op_(CompOp::NoOp) {
    left_.Init(false, 0, 0, nullptr);
    right_.Init(false, 0, 0, nullptr);
}
Re DefaultConditionFilter::Init(const ConDesc &left, const ConDesc &right, AttrType attr_type, CompOp comp_op) {
    if (attr_type < AttrType::Chars or attr_type > AttrType::Dates) {
        DebugPrint("DefaultConditionFilter:invalid condition with unsupported attribute type: %d\n", attr_type);
        return Re::InvalidArgument;
    }
    if (comp_op < CompOp::EqualTo or comp_op >= CompOp::NoOp) {
        DebugPrint("DefaultConditionFilter:invalid condition with unsupported compare operation: %d\n", comp_op);
        return Re::InvalidArgument;
    }
    left_ = left;
    right_ = right;
    attr_type_ = attr_type;
    comp_op_ = comp_op;
    return Re::Success;
}
Re DefaultConditionFilter::Init(Table &table, const Condition &condition) {
    const TableMeta &table_meta = table.GetTableMeta();
    ConDesc left, right;
    AttrType type_left = AttrType::Undefined, type_right = AttrType::Undefined;
    if (condition.left_is_attr == 1) {
        left.is_attr = true;
        const FieldMeta *field_left = table_meta.GetField(condition.left_attr.attr_name);
        if (field_left == nullptr) {
            DebugPrint("DefaultConditionFilter:No such field in condition. %s.%s\n", table.GetTableName(),
                       condition.left_attr.attr_name);
            return Re::SchemaFieldMissing;
        }
        left.Init(left.is_attr, field_left->GetLen(), field_left->GetOffset(), nullptr);
        type_left = field_left->GetAttrType();
    } else {
        left.Init(false, 0, 0, condition.left_value.data);
        type_left = condition.left_value.type;
    }
    if (condition.right_is_attr == 1) {
        right.is_attr = true;
        const FieldMeta *field_right = table_meta.GetField(condition.right_attr.attr_name);
        if (field_right == nullptr) {
            DebugPrint("DefaultConditionFilter:no such field in condition. %s.%s\n", table.GetTableName(),
                       condition.right_attr.attr_name);
            return Re::SchemaFieldMissing;
        }
        right.Init(right.is_attr, field_right->GetLen(), field_right->GetOffset(), nullptr);
        type_right = field_right->GetAttrType();
    } else {
        right.Init(false, 0, 0, condition.right_value.data);
        type_right = condition.right_value.type;
    }
    // 校验和转换
    // NOTE：这里没有实现不同类型的数据比较，比如整数跟浮点数之间的对比
    if (type_left != type_right)
        return Re::SchemaFieldTypeMismatch;
    return Init(left, right, type_left, condition.comp);
}
bool DefaultConditionFilter::Filter(class Record &rec) const {
    char *left_value = nullptr, *right_value = nullptr;
    if (left_.is_attr)// value
        left_value = reinterpret_cast<char *>(rec.GetData() + left_.attr_offset);
    else
        left_value = reinterpret_cast<char *>(left_.value);
    if (right_.is_attr)
        right_value = reinterpret_cast<char *>(rec.GetData() + right_.attr_offset);
    else
        right_value = reinterpret_cast<char *>(right_.value);
    int cmp_result = 0;
    switch (attr_type_) {
        case AttrType::Chars: {// 字符串都是定长的，直接比较
            cmp_result = strcmp(left_value, right_value);
        } break;
        case AttrType::Ints: {
            // 没有考虑大小端问题
            // 对int和float，要考虑字节对齐问题
            int left = *reinterpret_cast<int *>(left_value), right = *reinterpret_cast<int *>(right_value);
            cmp_result = left - right;
        } break;
        case AttrType::Floats: {
            float left = *reinterpret_cast<float *>(left_value), right = *reinterpret_cast<float *>(right_value);
            float result = left - right;
            cmp_result = result >= 0 ? ceil(result) : floor(result);
        } break;
        default: {
            assert(false);
        }
    }
    switch (comp_op_) {
        case CompOp::EqualTo:
            return cmp_result == 0;
        case CompOp::LessEqual:
            return cmp_result <= 0;
        case CompOp::NotEqual:
            return cmp_result != 0;
        case CompOp::LessThan:
            return cmp_result < 0;
        case CompOp::GreatEqual:
            return cmp_result >= 0;
        case CompOp::GreatThan:
            return cmp_result > 0;
        default:
            assert(false);
            break;
    }
    DebugPrint("DefaultConditionFilter:never should print this.\n");
    return cmp_result;// should not go here
}
CompositeConditionFilter::~CompositeConditionFilter() {
    if (memory_owner_) {
        delete[] filters_;
        filters_ = nullptr;
    }
}
Re CompositeConditionFilter::Init(const ConditionFilter *filters[], int filters_num, bool own_memory) {
    filters_ = filters;
    filters_num_ = filters_num;
    memory_owner_ = own_memory;
    return Re::Success;
}
Re CompositeConditionFilter::Init(const ConditionFilter *filters[], int filters_num) {
    return Init(filters, filters_num, false);
}
Re CompositeConditionFilter::Init(Table &table, const Condition *conditions, int conditions_num) {
    if (conditions_num == 0)
        return Re::Success;
    if (conditions == nullptr)
        return Re::InvalidArgument;
    Re r = Re::Success;
    ConditionFilter **condition_filters = new ConditionFilter *[conditions_num];
    for (int i = 0; i < conditions_num; i++) {
        DefaultConditionFilter *default_condition_filter = new DefaultConditionFilter();
        r = default_condition_filter->Init(table, conditions[i]);
        if (r != Re::Success) {
            delete default_condition_filter;
            for (int j = i - 1; j >= 0; j--) {
                delete condition_filters[j];
                condition_filters[j] = nullptr;
            }
            delete[] condition_filters;
            condition_filters = nullptr;
            return r;
        }
        condition_filters[i] = default_condition_filter;
    }
    return Init(const_cast<const ConditionFilter **>(condition_filters), conditions_num, true);
}
bool CompositeConditionFilter::Filter(class Record &rec) const {
    for (int i = 0; i < filters_num_; i++)
        if (!filters_[i]->Filter(rec))
            return false;
    return true;
}