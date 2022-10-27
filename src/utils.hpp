#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace ota::utils
{
    std::string str_join(const std::vector<std::string> &v, const std::string &delim = "");
    std::vector<std::string> str_split(const std::string &str, const std::string &delim);
    std::string str_trim(const std::string &str, const std::string &white = " \n\t\r");
}
