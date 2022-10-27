#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <optional>

#include "ota.hpp"

namespace ota
{
    struct mlfiPriv
    {
        std::vector<std::string> rcpts;
    };

    extern std::optional<OneTimeAddr> onetimeaddr;
    extern std::unordered_set<std::string> rcpts;
    extern struct smfiDesc smfilter;
}
