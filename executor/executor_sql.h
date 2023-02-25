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
    bool selectData(const std::string& column_name, const ConditionDescriptor_t& condition);
    bool insertRow(const std::vector<SqlValue_t>& value);
    bool deleteRow(const ConditionDescriptor_t& condition);

private:
    std::vector<TableColumnProperty_t>  vec_property_;
    std::vector<SqlRow_t>               vec_data_;

    static bool compare(const std::vector<SqlValue_t>& value, const ConditionDescriptor_t& condition);
    bool getColumnIndex(const std::string& column_name, uint32_t& index);
    bool verifyColumnProperty(const std::vector<TableColumnProperty_t>& vec_property);
    bool verifyRowData(const std::vector<SqlValue_t>& value);

};

class SqlDatabase_t
{
public:
    bool createTable(const std::string& tb_name);
    bool createTable(const std::string& tb_name, const std::vector<TableColumnProperty_t>& vec_column_property);
    bool dropTable(const std::string& tb_name);

    SqlTable_t* getTableByName(const std::string& tb_name);

private:
    std::map<std::string, SqlTable_t>  map_table_;
};

class SqlSupreme_t
{
public:
    bool createDatabase(const std::string& db_name);
    bool dropDatabase(const std::string& db_name);
    bool useDatabase(const std::string& db_name);

    SqlDatabase_t* getDatabaseInUse() { return p_db_in_use_; }

private:
    SqlDatabase_t*  p_db_in_use_ = nullptr;
    std::map<std::string, SqlDatabase_t>  map_database_;
};

} // namespace sql::exec