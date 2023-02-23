#pragma once 

#include "iostream"
#include "memory"

#include "parser/parser_fsm.h"
#include "executor/executor_dispatcher.h"
#include "def/sql_interface_def.h"
#include "common/lock_free_queue.h"

namespace sql
{

class SqlApp
{
public:

    bool init();
    void runApp();

private:
    void interrupt();

    // local variable
    std::shared_ptr<bool>        sp_is_running_;

    // components
    fsm::FsmParser               parser_;
    exec::SqlExecutorDispatcher  executor_;
    std::shared_ptr<LockFreeQueue<PacketCollection_t>>  sp_lfq_;
};

} // namespace sql
