#pragma once

#include "map"

#include "def/sql_interface_def.h"

namespace sql::exec
{

class SqlRow_t
{
public:

private:
    std::vector<SqlValue_t>  value_;
};

class SqlTable_t
{
public:
    bool selectData(std::string& column_name, ConditionDescriptor_t& condition);
    bool insertRow(std::vector<SqlValue_t>& value);
    bool deleteRow(ConditionDescriptor_t& condition);

private:
    std::string                         table_name_;
    std::vector<TableColumnProperty_t>  vec_property_;
    std::vector<SqlRow_t>               vec_data_;

    static bool compare(std::vector<SqlValue_t>& value, ConditionDescriptor_t& condition);
    bool getColumnIndex(std::string& column_name, uint32_t& index);
    bool verifyColumnProperty(std::vector<TableColumnProperty_t>& vec_property);
    bool verifyRowData(std::vector<SqlValue_t>& value);

};

class SqlDatabase_t
{
public:
    bool createTable(std::string& tb_name);
    bool createTable(std::string& tb_name, std::vector<TableColumnProperty_t>& vec_column_property);
    bool dropTable(std::string& tb_name);

    SqlTable_t* getTableByName(std::string& tb_name);

private:
    std::string db_name_;
    std::map<std::string, SqlTable_t>  map_table_;
};

class SqlSupreme_t
{
public:
    bool createDatabase(std::string& db_name);
    bool dropDatabase(std::string& db_name);
    bool useDatabase(std::string& db_name);

    SqlDatabase_t* getDatabaseInUse() { return p_db_in_use_; }

private:
    SqlDatabase_t*  p_db_in_use_ = nullptr;
    std::map<std::string, SqlDatabase_t>  map_database_;
};

} // namespace sql::exec