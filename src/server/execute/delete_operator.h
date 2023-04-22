#pragma once
#include "operator.h"
class Txn;
class DeleteStatement;
class DeleteOperator : public Operator
{
public:
    DeleteOperator() : delete_stmt_(nullptr), txn_(nullptr) {}
    DeleteOperator(DeleteStatement *delete_stmt, Txn *txn) : delete_stmt_(delete_stmt), txn_(txn) {}
    ~DeleteOperator() override = default;
    Re Init() override;
    Re Handle() override;
    Re Destroy() override;
    Tuple *GetCurrentTuple() override { return nullptr; }

private:
    DeleteStatement *delete_stmt_;
    Txn *txn_;
};
// class DeleteOperator : public Operator
// {
// public:
//   DeleteOperator(DeleteStmt *delete_stmt, Trx *trx)
//     : delete_stmt_(delete_stmt), trx_(trx)
//   {}
//   virtual ~DeleteOperator() = default;
//   RC open() override;
//   RC next() override;
//   RC close() override;
//   Tuple * current_tuple() override {
//     return nullptr;
//   }
//   //int tuple_cell_num() const override
//   //RC tuple_cell_spec_at(int index, TupleCellSpec &spec) const override
// private:
//   DeleteStmt *delete_stmt_ = nullptr;
//   Trx *trx_ = nullptr;
// };
