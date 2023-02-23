#include "executor/executor_sql.h"

namespace sql::exec
{
bool SqlSupreme_t::createDatabase(std::string& db_name)
{
    return true;
}

bool SqlSupreme_t::dropDatabase(std::string& db_name)
{
    return true;
}

bool SqlSupreme_t::useDatabase(std::string& db_name)
{
    return true;
}

} // namespace sql::exec
