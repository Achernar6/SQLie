#include "executor/executor_sql.h"

namespace sql::exec
{
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
        printf("Fail to drop database: Database \"%s\" doesn\'t exist\n", db_name.c_str());
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
        printf("Fail to use database: Database \"%s\" doesn\'t exist\n", db_name.c_str());
        return false;
    }

    p_db_in_use_ = &iter_db->second;
    printf("Use database \"%s\"\n", db_name.c_str());
    return true;
}

} // namespace sql::exec
