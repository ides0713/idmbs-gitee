#include "resolve_defs.h"
#include <cassert>
#include "../../common/common_defs.h"

void Statement::createStatement(Query *const query, Statement *&stmt) {
    switch (query->getScf()) {
        case ScfCreateTable:
            stmt = new CreateTableStatement(query);
            break;
        default:
            debugPrint("Statement:unrecognized query SCF\n");
            break;
    }
}

void SelectStatement::init(Query *query) {}

Re SelectStatement::handle(Query *query) { return Re::Success; }

void SelectStatement::destroy() {}

CreateTableStatement::CreateTableStatement(Query *query) :
        Statement(query->getScf()) { table_name_ = nullptr, attr_infos_num_ = 0, attr_infos_ = nullptr; }

void CreateTableStatement::init(Query *query) {
    CreateTableQuery *ctq = static_cast<CreateTableQuery *>(query);
    assert(this->getScf() == ScfCreateTable);
    attr_infos_num_ = ctq->getAttrNum();
    table_name_ = strNew(ctq->getRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = ctq->getAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i] = attr_infos[i];
}

Re CreateTableStatement::handle(Query *query) {
    return Re::Success;
}

void CreateTableStatement::destroy() {
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].destroy();
    delete[] attr_infos_;
}
