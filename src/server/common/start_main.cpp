#include "start_main.h"
#include "../parse/parse_defs.h"

Re StartMain::init(BaseMain *last_main)
{
    return Re::Success;
}

Re StartMain::handle()
{
    return Re::Success;
}

void StartMain::clear()
{
    if (sql_ != nullptr)
    {
        delete[] sql_;
        sql_ = nullptr;
    }
}

void StartMain::destroy()
{
    if (sql_ != nullptr)
    {
        delete[] sql_;
        sql_ = nullptr;
    }
}

char *StartMain::getSql()
{
    return sql_;
}

void StartMain::setSql(const char *sql)
{
    if (sql_ != nullptr)
    {
        delete[] sql_;
        sql_ = nullptr;
    }
    sql_ = strNew(sql);
    debugPrint("\nsetted sql is         %s\n\n", sql_);
}
