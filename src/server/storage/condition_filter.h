//
// Created by ubuntu on 4/1/23.
//
#pragma once

#include "../common/server_defs.h"
#include "../parse/parse_defs.h"
#include "../common/re.h"


class Record;

class Table;

struct ConDesc {
public:
    bool is_attr;
    int attr_length, attr_offset;
    void *value;
private:
};

class ConditionFilter {
public:
    virtual ~ConditionFilter();

    virtual bool filter(const class Record &rec) const = 0;
};

class DefaultConditionFilter : public ConditionFilter {
public:
    DefaultConditionFilter() : attr_type_(AttrType::Undefined), comp_op_(CompOp::NoOp) {}

    virtual ~DefaultConditionFilter();

    Re init(const ConDesc &left, const ConDesc &right, AttrType attr_type, CompOp comp_op);

    Re init(Table &table, const Condition &condition);

    virtual bool filter(const class Record &rec) const;

public:
    const ConDesc &getLeftConDesc() const { return left_; }

    const ConDesc &getRightConDesc() const { return right_; }

    CompOp getCompOp() const { return comp_op_; }

    AttrType getAttrType() const { return attr_type_; }

private:
    ConDesc left_, right_;
    AttrType attr_type_ = AttrType::Undefined;
    CompOp comp_op_ = CompOp::NoOp;
};

class CompositeConditionFilter : public ConditionFilter {
public:
    CompositeConditionFilter() : filters_(nullptr), filter_num_(0), memory_owner_(false) {}

    virtual ~CompositeConditionFilter();

    Re init(const ConditionFilter *filters[], int filter_num);

    Re init(Table &table, const Condition *conditions, int condition_num);

    virtual bool filter(const class Record &rec) const;

public:
    int getFilterNum() const { return filter_num_; }

    const ConditionFilter &filter(int index) const { return *filters_[index]; }

private:
    Re init(const ConditionFilter *filters[], int filter_num, bool own_memory);

private:
    const ConditionFilter **filters_;
    int filter_num_;
    bool memory_owner_; // filters_的内存是否由自己来控制
};