#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include <string_view>

namespace ota::utils
{
    std::string str_join(const std::vector<std::string> &v, const std::string &delim = "");
    std::vector<std::string> str_split(const std::string &str, const std::string &delim, const size_t max_split = SIZE_MAX);
    std::string str_trim(const std::string &str, const std::string &white = " \n\t\r");

    struct email_addr
    {
        std::string local_part;
        std::string user;
        std::optional<std::string> alias;
        std::string domain;

        std::string str() const
        {
            return local_part + '@' + domain;
        }
    };

    std::optional<email_addr> parse_email_addr(const std::string &addr);
}
