#include "executor/executor_dispatcher.h"

namespace sql::exec
{

bool SqlExecutorDispatcher::init(std::shared_ptr<LockFreeQueue<PacketCollection_t>>& sp_lfq)
{
    sp_lfq_ = sp_lfq;
    is_running_ = true;
    th_backend_ = std::thread(&SqlExecutorDispatcher::runBackend, this);

    return true;
}

bool SqlExecutorDispatcher::dispatch(PacketCollection_t& command)
{
    if (auto p_monostate = std::get_if<std::monostate>(&command))
    {
        printf("Empty data packet\n");
        return false;
    }
    else if (auto p_create_db = std::get_if<PacketCreateDatabase_t>(&command))
    {
        return handleCreateDatabase(*p_create_db);
    }
    else if (auto p_drop_db = std::get_if<PacketDropDatabase_t>(&command))
    {
        return handleDropDatabase(*p_drop_db);
    }
    else if (auto p_create_tb = std::get_if<PacketCreateTable_t>(&command))
    {
        return handleCreateTable(*p_create_tb);
    }
    else if (auto p_use_db = std::get_if<PacketUseDatabase_t>(&command))
    {
        return handleUseDatabase(*p_use_db);
    }
    else if (auto p_drop_tb = std::get_if<PacketDropTable_t>(&command))
    {
        return handleDropTable(*p_drop_tb);
    }
    else if (auto p_select = std::get_if<PacketSelect_t>(&command))
    {
        return handleSelect(*p_select);
    }
    else if (auto p_delect = std::get_if<PacketDelect_t>(&command))
    {
        return handleDelete(*p_delect);
    }
    else if (auto p_insert = std::get_if<PacketInsert_t>(&command))
    {
        return handleInsert(*p_insert);
    }
    else
    {
        printf("Unknown data packet\n");
        return false;
    }
}

void SqlExecutorDispatcher::runBackend()
{
    PacketCollection_t packet;
    while (is_running_)
    {
        if (!sp_lfq_->pop(packet)) continue;
        dispatch(packet);
    }
}

bool SqlExecutorDispatcher::handleCreateDatabase(const PacketCreateDatabase_t& packet)
{
    return sql_.createDatabase(packet.db_name);
}

bool SqlExecutorDispatcher::handleDropDatabase(const PacketDropDatabase_t& packet)
{
    return sql_.dropDatabase(packet.db_name);
}

bool SqlExecutorDispatcher::handleCreateTable(const PacketCreateTable_t& packet)
{
    if (sql_.getDatabaseInUse() == nullptr)
    {
        printf("Failed: no database in use\n");
        return false;
    }

    return sql_.getDatabaseInUse()->createTable(packet.table_name, packet.vec_column_property);
}

bool SqlExecutorDispatcher::handleUseDatabase(const PacketUseDatabase_t& packet)
{
    return sql_.useDatabase(packet.db_name);
}

bool SqlExecutorDispatcher::handleDropTable(const PacketDropTable_t& packet)
{
    if (sql_.getDatabaseInUse() == nullptr)
    {
        printf("Failed: no database in use\n");
        return false;
    }

    return sql_.getDatabaseInUse()->dropTable(packet.table_name);
}

bool SqlExecutorDispatcher::handleSelect(const PacketSelect_t& packet)
{
    if (sql_.getDatabaseInUse() == nullptr)
    {
        printf("Failed: no database in use\n");
        return false;
    }

    auto p_table_in_use = sql_.getDatabaseInUse()->getTableByName(packet.table_name);
    if (p_table_in_use == nullptr)
    {
        printf("Failed: table \"%s\" doesn\'t exist\n", packet.table_name.c_str());
        return false;
    }

    if (packet.condition.action == EnumConditionActionType::IDLE) return p_table_in_use->selectData(packet.column_name);
    else return p_table_in_use->selectData(packet.column_name, packet.condition);
}

bool SqlExecutorDispatcher::handleDelete(const PacketDelect_t& packet)
{
    if (sql_.getDatabaseInUse() == nullptr)
    {
        printf("Failed: no database in use\n");
        return false;
    }

    auto p_table_in_use = sql_.getDatabaseInUse()->getTableByName(packet.table_name);
    if (p_table_in_use == nullptr)
    {
        printf("Failed: table \"%s\" doesn\'t exist\n", packet.table_name.c_str());
        return false;
    }

    return p_table_in_use->deleteRow(packet.condition);
}

bool SqlExecutorDispatcher::handleInsert(const PacketInsert_t& packet)
{
    if (sql_.getDatabaseInUse() == nullptr)
    {
        printf("Failed: no database in use\n");
        return false;
    }

    auto p_table_in_use = sql_.getDatabaseInUse()->getTableByName(packet.table_name);
    if (p_table_in_use == nullptr)
    {
        printf("Failed: table \"%s\" doesn\'t exist\n", packet.table_name.c_str());
        return false;
    }

    return p_table_in_use->insertRow(packet.vec_value);
}

} // namespace sql::exec
