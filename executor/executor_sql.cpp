#include "executor/executor_sql.h"

#include "set"
#include "functional"

namespace sql::exec
{
bool SqlTable_t::selectData(const std::string& column_name)
{
    uint32_t column_index;
    if (!getColumnIndex(column_name, column_index))
    {
        printf("Fail to delete: column \"%s\" doesn\'t exist\n", column_name.c_str());
        return false;
    }
    
    std::function<void(const SqlValue_t&)> printer_wrapper;
    switch (vec_property_[column_index].value_type)
    {
        case EnumValueType::VALUE_TYPE_INT:
        {
            printer_wrapper = [](const SqlValue_t& value)
            {
                printf("  %d,\n", std::get<int32_t>(value));
            };
            break;
        }
        case EnumValueType::VALUE_TYPE_STRING:
        {
            printer_wrapper = [](const SqlValue_t& value)
            {
                printf("  %s,\n", std::get<std::string>(value).c_str());
            };
            break;
        }
        default:
        {
            printf("Fail to select: invalid column type\n");
            return false;
        }
    }

    printf("Select all data from column \"%s\":\n", column_name.c_str());
    auto row_iter = vec_data_.begin();
    while (row_iter != vec_data_.end())
    {
        printer_wrapper(*row_iter->data());
        row_iter ++;
    }

    return true;
}

bool SqlTable_t::selectData(const std::string& column_name, const ConditionDescriptor_t& condition)
{
    uint32_t column_index;
    if (!getColumnIndex(column_name, column_index))
    {
        printf("Fail to delete: column \"%s\" doesn\'t exist\n", column_name.c_str());
        return false;
    }

    std::function<bool(const SqlValue_t&, const SqlValue_t&, const EnumConditionActionType&)> printer_wrapper;
    switch (vec_property_[column_index].value_type)
    {
        case EnumValueType::VALUE_TYPE_INT:
        {
            printer_wrapper = [](const SqlValue_t& value, const SqlValue_t& anchor_value, const EnumConditionActionType& action)
            {
                auto int_value = std::get<int32_t>(value);
                auto int_anchor_value = std::get<int32_t>(anchor_value);
                if (compare<int32_t>(int_value, int_anchor_value, action))
                {
                    printf("  %d,\n", std::get<int32_t>(value));
                    return true;
                }
                return false;
            };
            break;
        }
        case EnumValueType::VALUE_TYPE_STRING:
        {
            printer_wrapper = [](const SqlValue_t& value, const SqlValue_t& anchor_value, const EnumConditionActionType& action)
            {
                auto str_value = std::get<std::string>(value);
                auto str_anchor_value = std::get<std::string>(anchor_value);
                if (compare<std::string>(str_value, str_anchor_value, action))
                {
                    printf("  %s,\n", std::get<std::string>(value).c_str());
                    return true;
                }
                return false;
            };
            break;
        }
        default:
        {
            printf("Fail to select: invalid column type\n");
            return false;
        }
    }

    printf("Select data from column \"%s\":\n", column_name.c_str());
    uint32_t cnt_row = 0;
    auto row_iter = vec_data_.begin();
    while (row_iter != vec_data_.end())
    {
        if (printer_wrapper(*row_iter->data(), condition.anchor_val, condition.action))
        {
            cnt_row ++;
        }
        row_iter ++;
    }
    printf("%d row(s) selected\n", cnt_row);

    return true;
}

bool SqlTable_t::insertRow(const std::vector<std::string>& value)
{
    auto value_ = std::vector<SqlValue_t>{};
    if (!verifyRowData(value, value_))
    {
        printf("Fail to insert: data verification failed\n");
        return false;
    }

    vec_data_.emplace_back(value_);
    return true;
}

bool SqlTable_t::deleteRow(const ConditionDescriptor_t& condition)
{
    uint32_t column_index;
    if (!getColumnIndex(condition.column_name, column_index))
    {
        printf("Fail to delete: column \"%s\" doesn\'t exist\n", condition.column_name.c_str());
        return false;
    }

    std::function<bool(const SqlValue_t&, const SqlValue_t&, const EnumConditionActionType&)> compare_wrapper;
    switch (vec_property_[column_index].value_type)
    {
        case EnumValueType::VALUE_TYPE_INT:
        {
            compare_wrapper = [](const SqlValue_t& value, const SqlValue_t& anchor_value, const EnumConditionActionType& action)
            {
                auto int_value = std::get<int32_t>(value);
                auto int_anchor_value = std::get<int32_t>(anchor_value);
                return compare<int32_t>(int_value, int_anchor_value, action);
            };
            break;
        }
        case EnumValueType::VALUE_TYPE_STRING:
        {
            compare_wrapper = [](const SqlValue_t& value, const SqlValue_t& anchor_value, const EnumConditionActionType& action)
            {
                auto str_value = std::get<std::string>(value);
                auto str_anchor_value = std::get<std::string>(anchor_value);
                return compare<std::string>(str_value, str_anchor_value, action);
            };
            break;
        }
        default:
        {
            printf("Fail to delete: invalid column type\n");
            return false;
        }
    }

    uint32_t cnt_row = 0;
    auto row_iter = vec_data_.begin();
    while (row_iter != vec_data_.end())
    {
        if (compare_wrapper(*row_iter->data(), condition.anchor_val, condition.action))
        {
            row_iter = vec_data_.erase(row_iter);
            cnt_row ++;
        }
        else
        {
            row_iter ++;
        }
    }

    printf("%d row(s) deleted\n", cnt_row);
    return true;
}

bool SqlTable_t::getColumnIndex(const std::string& column_name, uint32_t& index)
{
    for (uint16_t index_ = 0; index_ < vec_property_.size(); index_ ++)
    {
        if (column_name == vec_property_[index_].column_name)
        {
            index = index_;
            return true;
        }
    }

    return false;
}

bool SqlTable_t::verifyRowData(const std::vector<std::string>& raw_value, std::vector<SqlValue_t>& value)
{
    if (raw_value.empty() || raw_value.size() != vec_property_.size()) return false;

    value.clear();
    for (uint16_t index = 0; index < value.size(); index ++)
    {
        switch (vec_property_[index].value_type)
        {
            case EnumValueType::VALUE_TYPE_INT:
            {
                try
                {
                    int32_t num_val = std::stoi(raw_value[index]);
                    if (vec_property_[index].is_primary)
                    {
                        for (uint16_t index_ = 0; index_ < vec_data_.size(); index_ ++)
                        {
                            if (num_val == std::get<int32_t>(vec_data_[index_][index])) return false;  // duplicate primary key
                        }
                    }
                    value.emplace_back(SqlValue_t{num_val});
                }
                catch (std::invalid_argument const &exception) { return false; }
                catch (std::out_of_range const &exception) { return false; }
                break;
            }
            case EnumValueType::VALUE_TYPE_STRING:
            {
                if (vec_property_[index].is_primary)
                {
                    for (uint16_t index_ = 0; index_ < vec_data_.size(); index_ ++)
                    {
                        if (raw_value[index] == std::get<std::string>(vec_data_[index_][index])) return false;  // duplicate primary key
                    }
                }
                value.emplace_back(SqlValue_t{raw_value[index]});
                break;
            }
            default: 
                return false;
        }
    }

    return true;
}

bool SqlDatabase_t::createTable(const std::string& tb_name, const std::vector<TableColumnProperty_t>& vec_column_property)
{
    auto iter_tb = map_table_.find(tb_name);
    if (iter_tb != map_table_.end())
    {
        printf("Fail to create table: table \"%s\" exists\n", tb_name.c_str());
        return false;
    }

    auto new_table = SqlTable_t{};
    
    // verify row property
    uint16_t num_primary = 0;
    std::set<std::string> set_name;

    for (auto& column_property : vec_column_property)
    {
        if (column_property.value_type == EnumValueType::VALUE_TYPE_IDLE)
        {
            printf("Fail to create table: table \"%s\" has invalid column type\n", tb_name.c_str());
            return false;
        }

        if (set_name.find(column_property.column_name) != set_name.end())
        {
            printf("Fail to create table: table \"%s\" has duplicate column name \"%s\"\n", tb_name.c_str(), column_property.column_name.c_str());
            return false;
        }

        set_name.emplace(column_property.column_name);
        num_primary += (column_property.is_primary) ? 1 : 0;
    }

    if (num_primary == 0)
    {
        printf("Fail to create table: table \"%s\" has no primary key\n", tb_name.c_str());
        return false;
    }
    else if (num_primary > 1)
    {
        printf("Fail to create table: table \"%s\" has excessive primary key(s)[%d]\n", tb_name.c_str(), num_primary);
        return false;
    }

    new_table.setProperty(vec_column_property);
    map_table_.emplace(tb_name, new_table);
    printf("Create table \"%s\"\n", tb_name.c_str());
    return true;
}

bool SqlDatabase_t::dropTable(const std::string& tb_name)
{
    auto iter_tb = map_table_.find(tb_name);
    if (iter_tb == map_table_.end())
    {
        printf("Fail to drop table: table \"%s\" doesn\'t exist\n", tb_name.c_str());
        return false;
    }

    map_table_.erase(tb_name);
    printf("Drop table \"%s\"\n", tb_name.c_str());

    return true;
}

SqlTable_t* SqlDatabase_t::getTableByName(const std::string& tb_name)
{
    auto iter_tb = map_table_.find(tb_name);
    return (iter_tb == map_table_.end()) ? nullptr : &iter_tb->second;
}

bool SqlSupreme_t::createDatabase(const std::string& db_name)
{
    auto iter_db = map_database_.find(db_name);
    if (iter_db != map_database_.end())
    {
        printf("Fail to create database: database \"%s\" exists\n", db_name.c_str());
        return false;
    }

    map_database_.emplace(db_name, SqlDatabase_t{});
    printf("Create database \"%s\"\n", db_name.c_str());

    return true;
}

bool SqlSupreme_t::dropDatabase(const std::string& db_name)
{
    auto iter_db = map_database_.find(db_name);
    if (iter_db == map_database_.end())
    {
        printf("Fail to drop database: database \"%s\" doesn\'t exist\n", db_name.c_str());
        return false;
    }

    map_database_.erase(db_name);
    printf("Drop database \"%s\"\n", db_name.c_str());
    return true;
}

bool SqlSupreme_t::useDatabase(const std::string& db_name)
{
    auto iter_db = map_database_.find(db_name);
    if (iter_db == map_database_.end())
    {
        printf("Fail to use database: database \"%s\" doesn\'t exist\n", db_name.c_str());
        return false;
    }

    p_db_in_use_ = &iter_db->second;
    printf("Use database \"%s\"\n", db_name.c_str());
    return true;
}

} // namespace sql::exec
