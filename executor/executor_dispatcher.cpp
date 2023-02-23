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
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketDropDatabase_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketCreateTable_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketUseDatabase_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketDropTable_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketSelect_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketDelect_t>(&command))
    {
        return true;
    }
    else if (auto p_create_db = std::get_if<PacketInsert_t>(&command))
    {
        return true;
    }

    printf("Unknown data packet\n");
    return false;
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
    return true;
}

bool SqlExecutorDispatcher::handleDropDatabase(const PacketDropDatabase_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleCreateTable(const PacketCreateTable_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleUseDatabase(const PacketUseDatabase_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleDropTable(const PacketDropTable_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleSelect(const PacketSelect_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleDelete(const PacketDelect_t& packet)
{
    return true;
}

bool SqlExecutorDispatcher::handleInsert(const PacketInsert_t& packet)
{
    return true;
}

} // namespace sql::exec
