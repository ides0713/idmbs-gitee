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

RE SelectStatement::handle(Query *query) {}

void SelectStatement::destroy() {}

CreateTableStatement::CreateTableStatement(Query *query) : Statement(query->getSCF()) {}

void CreateTableStatement::initialize(Query *query) {
    CreateTableQuery *ctq = static_cast<CreateTableQuery *>(query);
    assert(this->getSCF() == SCF_CREATE_TABLE);
    attr_infos_num_ = ctq->getAttrNum();
    table_name_ = strnew(ctq->getRelName());
    attr_infos_ = new AttrInfo[attr_infos_num_];
    AttrInfo *attr_infos = ctq->getAttrs();
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i] = attr_infos[i];
}

RE CreateTableStatement::handle(Query *query) {
    return RE::SUCCESS;
}

void CreateTableStatement::destroy() {
    delete[] table_name_;
    for (int i = 0; i < attr_infos_num_; i++)
        attr_infos_[i].destroy();
    delete[] attr_infos_;
}
