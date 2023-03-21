#include "resolve_defs.h"
#include <assert.h>

void Statement::createStatement(Query *const query, Statement *&stmt) {
    switch (query->getSCF()) {
        case SCF_CREATE_TABLE:
            stmt = new CreateTableStatement(query);
            break;
        default:
            printf("Statement:unrecognized query SCF\n");
            break;
    }
}

void SelectStatement::initialize(Query *query) {}

void SelectStatement::handle(Query *query) {}

void SelectStatement::destroy() {}

CreateTableStatement::CreateTableStatement(Query *query) : Statement(query->getSCF()) {}

void CreateTableStatement::initialize(Query *query) {
    CreateTableQuery *cq = static_cast<CreateTableQuery *>(query);
    assert(this->getSCF() == SCF_CREATE_TABLE);
    attr_infos_num_ = cq->getAttrNum();
    table_name_ = strnew(cq->getRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = cq->getAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i] = attr_infos[i];
}

void CreateTableStatement::handle(Query *query) {}

void CreateTableStatement::destroy() {
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].destroy();
    delete[] attr_infos_;
}
