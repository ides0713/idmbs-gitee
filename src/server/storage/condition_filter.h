//
// Created by ubuntu on 4/1/23.
//
#pragma once
#include "../common/re.h"       // for Re
#include "../parse/parse_defs.h"// for AttrType, CompOp, NoOp, Undefined
class Table;
/// @brief conditon descriptor
struct ConDesc {
public:
    bool is_attr;
    int attr_length, attr_offset;
    void *value;

public:
    ConDesc() : is_attr(false), attr_length(-1), attr_offset(-1), value(nullptr) {}
    void Init(bool is_attr, int attr_length, int attr_offset, void *value);
};
class ConditionFilter
{
public:
    virtual ~ConditionFilter() = default;
    virtual bool Filter(class Record &rec) const = 0;
};
class DefaultConditionFilter : public ConditionFilter
{
public:
    DefaultConditionFilter();
    ~DefaultConditionFilter() override = default;
    Re Init(const ConDesc &left, const ConDesc &right, AttrType attr_type, CompOp comp_op);
    Re Init(Table &table, const Condition &condition);
    bool Filter(class Record &rec) const override;

public:
    const ConDesc &GetLeftConDesc() const { return left_; }
    const ConDesc &GetRightConDesc() const { return right_; }
    CompOp GetCompOp() const { return comp_op_; }
    AttrType GetAttrType() const { return attr_type_; }

private:
    ConDesc left_, right_;
    AttrType attr_type_;
    CompOp comp_op_;
};
class CompositeConditionFilter : public ConditionFilter
{
public:
    CompositeConditionFilter() : filters_(nullptr), filters_num_(0), memory_owner_(false) {}
    ~CompositeConditionFilter() override;
    Re Init(const ConditionFilter *filters[], int filters_num);
    Re Init(Table &table, const Condition *conditions, int conditions_num);
    bool Filter(class Record &rec) const override;

public:
    int GetFilterNum() const { return filters_num_; }
    const ConditionFilter &GetFilter(int index) const { return *filters_[index]; }

private:
    Re Init(const ConditionFilter *filters[], int filters_num, bool own_memory);

private:
    const ConditionFilter **filters_;
    int filters_num_;
    bool memory_owner_;// filters_的内存是否由自己来控制
};