#pragma once

#include "expression.h"
#include "../parse/parse_defs.h"
#include "../common/re.h"

class DataBase;

class Table;

class FieldMeta;


class FilterUnit {
public:
    FilterUnit() : comp_(CompOp::NoOp), left_(nullptr), right_(nullptr) {}

    ~FilterUnit();

    void setComp(CompOp cmp) { comp_ = cmp; }

    void setLeft(Expression *expr) { left_ = expr; }

    void setRight(Expression *expr) { right_ = expr; }

    [[nodiscard]] CompOp getComp() const { return comp_; }

    [[nodiscard]] Expression *getLeft() const { return left_; }

    [[nodiscard]] Expression *getRight() const { return right_; }

private:
    CompOp comp_;
    Expression *left_;
    Expression *right_;
};

class Filter {
public:
    Filter() = default;

    ~Filter();

    [[nodiscard]] const std::vector<FilterUnit *> &getFilterUnits() const { return filter_units_; }

public:
    ///@brief create filter succeed means value or field in the condition is comparable and exists in table at least now
    static Re createFilter(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                           int conditions_num, const Condition *conditions, Filter *&filter);

private:
    std::vector<FilterUnit *> filter_units_;
private:
    ///@brief filter unit is filter of its corresponding condition,filter is composed of many filter unit
    static Re createFilterUnit(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                               const Condition &condition, FilterUnit *&filter_unit);
};