#include "table.h"
#include "../parse/parse_defs.h"
Table::Table()
{
    table_name_=nullptr;
    table_meta_=nullptr;
}

Table::Table(const char *name)
{
    table_name_=strnew(name);
    table_meta_=nullptr;
}

