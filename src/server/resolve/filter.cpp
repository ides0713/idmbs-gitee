#include "filter.h"
#include "../storage/database.h"
#include "../storage/field.h"
#include "../storage/table.h"
Re GetTableAndField(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                    const RelAttr &attr, Table *&table, const FieldMeta *&field) {
    if (attr.rel_name == nullptr)
        table = default_table;
    else if (tables != nullptr) {
        auto iter = tables->find(std::string(attr.rel_name));
        if (iter != tables->end())
            table = iter->second;
    } else
        table = db->GetTable(std::string(attr.rel_name));
    if (nullptr == table) {
        DebugPrint("FilterFunc:no such table: attr.relation_name: %s\n", attr.rel_name);
        return Re::SchemaTableNotExist;
    }
    field = table->GetTableMeta().GetField(attr.attr_name);
    if (field == nullptr) {
        DebugPrint("FilterFunc:no such field in table: table %s, field %s\n", table->GetTableName(), attr.attr_name);
        table = nullptr;
        return Re::SchemaFieldNotExist;
    }
    return Re::Success;
}
void SetTempType(Expression *expr, AttrType &type) {
    if (expr->GetExprType() == ExprType::Field) {
        auto fe = static_cast<FieldExpression *>(expr);
        type = fe->GetField().GetAttrType();
    } else {
        auto ve = static_cast<ValueExpression *>(expr);
        TupleUnit temp;
        ve->GetTupleUnit(temp);
        type = temp.GetAttrType();
    }
}
bool IsComparable(Expression *a, Expression *b) {
    if (a->GetExprType() == ExprType::None or b->GetExprType() == ExprType::None) {
        printf("====== filter.cpp\n");
        return false;
    } else {
        AttrType a_type = AttrType::Undefined, b_type = AttrType::Undefined;
        SetTempType(a, a_type);
        SetTempType(b, b_type);
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
Re Filter::CreateFilter(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                        int conditions_num, const Condition *conditions, Filter *&filter) {
    filter = nullptr;
    Filter *temp_filter = new Filter;
    for (int i = 0; i < conditions_num; i++) {
        FilterUnit *filter_unit = nullptr;
        Re r = CreateFilterUnit(db, default_table, tables, conditions[i], filter_unit);
        if (r != Re::Success) {
            delete filter_unit;
            DebugPrint("Filter:failed to create filter,create filter unit of condition with index:%d failed\n", i);
            return r;
        }
        temp_filter->filter_units_.push_back(filter_unit);
    }
    filter = temp_filter;
    return Re::Success;
}
Re Filter::CreateFilterUnit(DataBase *db, Table *default_table, std::unordered_map<std::string, Table *> *tables,
                            const Condition &condition, FilterUnit *&filter_unit) {
    CompOp temp_comp = condition.comp;
    if (temp_comp < CompOp::EqualTo or temp_comp >= CompOp::NoOp) {
        DebugPrint("Filter:invalid compare operator:%d\n", temp_comp);
        return Re::InvalidArgument;
    }
    Expression *left = nullptr, *right = nullptr;
    if (condition.left_is_attr) {
        Table *c_table = nullptr;
        const FieldMeta *c_field = nullptr;
        Re r = GetTableAndField(db, default_table, tables, condition.left_attr, c_table, c_field);
        if (r != Re::Success) {
            DebugPrint("Filter:can not find attr's corresponding table and field\n");
            return r;
        }
        left = new FieldExpression(c_table, c_field);
    } else
        left = new ValueExpression(condition.left_value);
    if (condition.right_is_attr) {
        Table *c_table = nullptr;
        const FieldMeta *c_field = nullptr;
        Re r = GetTableAndField(db, default_table, tables, condition.right_attr, c_table, c_field);
        if (r != Re::Success) {
            DebugPrint("Filter:can not find attr's corresponding table and field\n");
            return r;
        }
        right = new FieldExpression(c_table, c_field);
    } else
        right = new ValueExpression(condition.right_value);
    if (IsComparable(left, right)) {
        filter_unit = new FilterUnit;
        filter_unit->SetComp(temp_comp);
        filter_unit->SetLeft(left);
        filter_unit->SetRight(right);
        return Re::Success;
    }
    // not comparable
    return Re::GenericError;
}
