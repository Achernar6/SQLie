#pragma once

#include "map"

#include "def/sql_interface_def.h"

namespace sql::exec
{

template <typename SqlType>
bool compare(const SqlType& value, const SqlType& anchor_value, const EnumConditionActionType& action)
{
    switch (action)
    {
        case EnumConditionActionType::LT:   return value <  anchor_value;
        case EnumConditionActionType::LTEQ: return value <= anchor_value;
        case EnumConditionActionType::EQ:   return value == anchor_value;
        case EnumConditionActionType::GTEQ: return value >= anchor_value;
        case EnumConditionActionType::GT:   return value >  anchor_value;
        default: return false;
    }
}

class SqlTable_t
{
public:
    bool selectData(const std::string& column_name);
    bool selectData(const std::string& column_name, const ConditionDescriptor_t& condition);
    bool insertRow(const std::vector<std::string>& value);
    bool deleteRow(const ConditionDescriptor_t& condition);
    inline void setProperty(const std::vector<TableColumnProperty_t>& vec_column_property) { vec_property_ = vec_column_property; }

private:
    std::vector<TableColumnProperty_t>    vec_property_;
    std::vector<std::vector<SqlValue_t>>  vec_data_;

    bool getColumnIndex(const std::string& column_name, uint32_t& index);
    bool verifyRowData(const std::vector<std::string>& raw_value, std::vector<SqlValue_t>& value);

};

class SqlDatabase_t
{
public:
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