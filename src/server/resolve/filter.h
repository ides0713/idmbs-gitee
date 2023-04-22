#pragma once
#include "../common/re.h"
#include "../parse/parse_defs.h"
#include "expression.h"
class DataBase;
class Table;
class FieldMeta;
class FilterUnit
{
public:
    FilterUnit() : comp_(CompOp::NoOp), left_(nullptr), right_(nullptr) {}
    ~FilterUnit();
    void SetComp(CompOp cmp) { comp_ = cmp; }
    void SetLeft(Expression *expr) { left_ = expr; }
    void SetRight(Expression *expr) { right_ = expr; }
    [[nodiscard]] CompOp GetComp() const { return comp_; }
    [[nodiscard]] Expression *GetLeft() const { return left_; }
    [[nodiscard]] Expression *GetRight() const { return right_; }

private:
    CompOp comp_;
    Expression *left_;
    Expression *right_;
};
class Filter
{
public:
    Filter() = default;
    ~Filter();
    [[nodiscard]] const std::vector<FilterUnit *> &GetFilterUnits() const { return filter_units_; }

public:
    ///@brief create filter succeed means value or field in the condition is comparable and exists in table at least now
    static Re CreateFilter(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                           int conditions_num, const Condition *conditions, Filter *&filter);

private:
    std::vector<FilterUnit *> filter_units_;

private:
    ///@brief filter unit is filter of its corresponding condition,filter is composed of many filter unit
    static Re CreateFilterUnit(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                               const Condition &condition, FilterUnit *&filter_unit);
};