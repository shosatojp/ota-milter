#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <optional>
#include <memory>

#include "ota.hpp"

namespace ota
{
    struct mlfiPriv
    {
        std::vector<std::string> rcpts;
    };

    extern std::optional<std::unique_ptr<OneTimeAddr>> onetimeaddr;
    extern std::unordered_set<std::string> rcpts;
    extern struct smfiDesc smfilter;
}
