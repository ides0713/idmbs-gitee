#include "resolve_defs.h"
void Statement::createStatement(Query *const query, Statement *&stmt)
{
    switch (query->getSCF())
    {
    case SCF_CREATE_TABLE:
        stmt = new CreateTableStatement(query);
        break;
    default:
        printf("unrecognized query SCF\n");
        break;
    }
}

void SelectStatement::initialize(Query* query)
{
}
void SelectStatement::handle(Query *query)
{
}
void SelectStatement::destroy()
{
}
void CreateTableStatement::initialize(Query* query)
{
}
void CreateTableStatement::handle(Query *query)
{
}
void CreateTableStatement::destroy()
{
}
