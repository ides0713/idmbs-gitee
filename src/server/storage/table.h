#pragma once
class TableMeta
{
};
class Table
{
public:
    Table();
    Table(const char *name);
    void initialize(const char * name);
    void initialize(const char *name,const TableMeta* meta);
private:
    char *table_name_;
    TableMeta *table_meta_;
};
class TableManager
{
};