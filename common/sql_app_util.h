#pragma once

#include "iostream"
#include "vector"

namespace sql
{

size_t splitArgument(std::string& line, std::vector<std::string>& params);

}