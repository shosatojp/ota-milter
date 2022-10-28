#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <optional>
#include <string_view>

#include "utils.hpp"

namespace ota::utils
{
    std::string str_join(const std::vector<std::string> &v, const std::string &delim)
    {
        std::ostringstream ss;
        for (size_t i = 0, len = v.size(); i < len; i++)
        {
            if (i != 0)
                ss << delim;
            ss << v[i];
        }
        return ss.str();
    }

    std::vector<std::string> str_split(const std::string &str, const std::string &delim, const size_t max_split)
    {
        std::vector<std::string> v;
        size_t start = 0, end = 0;
        for (size_t i = 0; i < max_split && (end = str.find(delim, start)) != std::string::npos; i++)
        {
            v.push_back(str.substr(start, end - start));
            start = end + delim.size();
        }
        v.push_back(str.substr(start));
        return v;
    }

    std::string str_trim(const std::string &str, const std::string &white)
    {
        const size_t start = str.find_first_not_of(white),
                     end = str.find_last_not_of(white);
        return start < end ? str.substr(start, end - start + 1) : "";
    }

    std::optional<email_addr> parse_email_addr(const std::string &addr)
    {
        const auto fragments = str_split(addr, "@", 1);
        if (fragments.size() != 2)
            return std::nullopt;
        const std::string local_part{fragments[0]};

        struct email_addr result = {
            .local_part = local_part,
            .user = local_part,
            .alias = std::nullopt,
            .domain = std::string{fragments[1]},
        };

        const auto user_alias = str_split(local_part, "+", 1);
        switch (user_alias.size())
        {
        case 2:
            result.user = user_alias[0];
            result.alias = user_alias[1];
            break;
        case 1:
            result.user = local_part;
            result.alias = std::nullopt;
            break;
        default:
            return std::nullopt;
        }

        return result;
    }
}
