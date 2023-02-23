#include "vector"

#include "common/sql_app.h"
#include "common/sql_app_util.h"
#include "3rd/cpp-linenoise/linenoise.hpp"

namespace sql
{

bool SqlApp::init()
{
    sp_lfq_ = std::make_shared<LockFreeQueue<PacketCollection_t>>(LFQ_MAX_SIZE);
    sp_is_running_ = std::make_shared<bool>(true);

    if (!parser_.init(sp_lfq_, sp_is_running_))
    {
        printf("Parser initialization failed\n");
        return false;
    }

    if (!executor_.init(sp_lfq_))
    {
        printf("Executor initialization failed\n");
        return false;
    }
    
    printf("Sql terminal init complete\n");
    
    return true;
}

void SqlApp::runApp()
{
    if (*sp_is_running_)
    {
        const auto history_path = "linenoiseHistory.txt";
        linenoise::LoadHistory(history_path);

        while (*sp_is_running_)
        {
            std::string line;
            if (linenoise::Readline("\033[32mSql\x1b[0m> ", line)) interrupt();

            std::vector<std::string> params;
            if (splitArgument(line, params))
            {
                parser_.parseInput(params);
            }

            linenoise::AddHistory(line.c_str());
        }
        
        linenoise::SaveHistory(history_path);
    }
    
    printf("Sql terminal exit\n");
}

void SqlApp::interrupt()
{
    *sp_is_running_ = false;
}

} // namespace sql
