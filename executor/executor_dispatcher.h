#pragma once

#include "memory"
#include "thread"
#include "pthread.h"

#include "def/sql_interface_def.h"
#include "common/lock_free_queue.h"
#include "executor/executor_sql.h"

namespace sql::exec
{
class SqlExecutorDispatcher
{
public:
    bool init(std::shared_ptr<LockFreeQueue<PacketCollection_t>>& sp_lfq);

private:
    bool dispatch(PacketCollection_t& command);
    void runBackend();

    bool handleCreateDatabase(const PacketCreateDatabase_t& packet);
    bool handleDropDatabase(const PacketDropDatabase_t& packet);
    bool handleCreateTable(const PacketCreateTable_t& packet);
    bool handleUseDatabase(const PacketUseDatabase_t& packet);
    bool handleDropTable(const PacketDropTable_t& packet);
    bool handleSelect(const PacketSelect_t& packet);
    bool handleDelete(const PacketDelect_t& packet);
    bool handleInsert(const PacketInsert_t& packet);

    bool         is_running_;
    std::thread  th_backend_;
    std::shared_ptr<LockFreeQueue<PacketCollection_t>>  sp_lfq_;

};

} // namespace sql::exec
