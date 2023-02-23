#include "common/sql_app_util.h"

namespace sql
{

size_t splitArgument(std::string& line, std::vector<std::string>& params)
{
    std::string word;
    size_t index = 0;
    for (; index < line.size(); index ++)
    {
        char& _char = line[index];

        if (_char == '\n' || _char == '\t') continue;
        if (_char != ' ') 
        {
            word.append(1, _char);
        }
        else if (!word.empty())  // separate word
        {
            params.emplace_back(std::move(word));
        }
    }

    if (!word.empty()) // last word
    {
        params.emplace_back(word);
    }

    return index;
}

} // namespace sql
