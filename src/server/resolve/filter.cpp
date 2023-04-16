#include "filter.h"
#include "../storage/field.h"
#include "../storage/table.h"
#include "../storage/database.h"

Re getTableAndField(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                    const RelAttr &attr, Table *&table, const FieldMeta *&field) {
    if (attr.rel_name == nullptr)
        table = default_table;
    else if (tables != nullptr) {
        auto iter = tables->find(std::string(attr.rel_name));
        if (iter != tables->end())
            table = iter->second;
    } else
        table = db->getTable(std::string(attr.rel_name));
    if (nullptr == table) {
        debugPrint("FilterFunc:no such table: attr.relation_name: %s\n", attr.rel_name);
        return Re::SchemaTableNotExist;
    }
    field = table->getTableMeta().getField(attr.attr_name);
    if (field == nullptr) {
        debugPrint("FilterFunc:no such field in table: table %s, field %s\n",
                   table->getTableName().c_str(), attr.attr_name);
        table = nullptr;
        return Re::SchemaFieldNotExist;
    }
    return Re::Success;
}

void setTempType(Expression *expr, AttrType &type) {
    if (expr->getExprType() == ExprType::Field) {
        auto fe = static_cast<FieldExpression *>(expr);
        type = fe->getField().getAttrType();
    } else {
        auto ve = static_cast<ValueExpression *>(expr);
        TupleUnit temp;
        ve->getTupleUnit(temp);
        type = temp.getAttrType();
    }
}

bool isComparable(Expression *a, Expression *b) {
    if (a->getExprType() == ExprType::None or b->getExprType() == ExprType::None) {
        printf("====== filter.cpp\n");
        return false;
    } else {
        AttrType a_type = AttrType::Undefined, b_type = AttrType::Undefined;
        setTempType(a, a_type);
        setTempType(b, b_type);
        if (a_type == AttrType::Undefined or b_type == AttrType::Undefined)
            return false;
        if (a_type == b_type)
            return true;
        if ((a_type == AttrType::Ints and b_type == AttrType::Floats) or
            (a_type == AttrType::Floats and b_type == AttrType::Ints))
            return true;
        return false;
    }
}

FilterUnit::~FilterUnit() {
    delete left_;
    left_ = nullptr;
    delete right_;
    right_ = nullptr;
}

Filter::~Filter() {
    for (auto unit: filter_units_)
        delete unit;
    filter_units_.clear();
}

Re Filter::createFilter(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                        int conditions_num, const Condition *conditions, Filter *&filter) {
    filter = nullptr;
    Filter *temp_filter = new Filter;
    for (int i = 0; i < conditions_num; i++) {
        FilterUnit *filter_unit = nullptr;
        Re r = createFilterUnit(db, default_table, tables, conditions[i], filter_unit);
        if (r != Re::Success) {
            delete filter_unit;
            debugPrint("Filter:failed to create filter,create filter unit of condition with index:%d failed\n", i);
            return r;
        }
        temp_filter->filter_units_.push_back(filter_unit);
    }
    filter = temp_filter;
    return Re::Success;
}

Re Filter::createFilterUnit(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                            const Condition &condition, FilterUnit *&filter_unit) {
    CompOp temp_comp = condition.comp;
    if (temp_comp < CompOp::EqualTo or temp_comp >= CompOp::NoOp) {
        debugPrint("Filter:invalid compare operator:%d\n", temp_comp);
        return Re::InvalidArgument;
    }
    Expression *left = nullptr, *right = nullptr;
    if (condition.left_is_attr) {
        Table *c_table = nullptr;
        const FieldMeta *c_field = nullptr;
        Re r = getTableAndField(db, default_table, tables, condition.left_attr, c_table, c_field);
        if (r != Re::Success) {
            debugPrint("Filter:can not find attr's corresponding table and field\n");
            return r;
        }
        left = new FieldExpression(c_table, c_field);
    } else
        left = new ValueExpression(condition.left_value);
    if (condition.right_is_attr) {
        Table *c_table = nullptr;
        const FieldMeta *c_field = nullptr;
        Re r = getTableAndField(db, default_table, tables, condition.right_attr, c_table, c_field);
        if (r != Re::Success) {
            debugPrint("Filter:can not find attr's corresponding table and field\n");
            return r;
        }
        right = new FieldExpression(c_table, c_field);
    } else
        right = new ValueExpression(condition.right_value);
    if (isComparable(left, right)) {
        filter_unit = new FilterUnit;
        filter_unit->setComp(temp_comp);
        filter_unit->setLeft(left);
        filter_unit->setRight(right);
        return Re::Success;
    }
    //not comparable
    return Re::GenericError;
}
