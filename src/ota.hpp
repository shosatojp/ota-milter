#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <sstream>
#include <chrono>
#include <optional>
#include <random>
#include <regex>
#include <thread>

#include "utils.hpp"

namespace ota
{
    using utils::email_addr;

    struct OneTimeAddrEntry
    {
        std::chrono::time_point<std::chrono::system_clock> expires_at;
        email_addr realrcpt;
    };

    class OneTimeAddr
    {
    private:
        std::thread cleanup_thread;
        std::unordered_map<std::string, OneTimeAddrEntry> tmpaddrs;
        std::random_device seed_gen;
        std::mt19937 random_engine;
        std::vector<email_addr> rcpts;

    public:
        std::chrono::seconds expires_in;
        OneTimeAddr(const OneTimeAddr &ota) = delete;
        OneTimeAddr(const OneTimeAddr &&ota) = delete;
        OneTimeAddr(const std::vector<email_addr> &rcpts, const std::chrono::seconds expires_in);
        ~OneTimeAddr();
        OneTimeAddr operator=(const OneTimeAddr &ota) = delete;
        OneTimeAddr operator=(const OneTimeAddr &&ota) = delete;
        std::string create(const email_addr &realrcpt);
        std::optional<email_addr> match(const std::string &addr);
        std::optional<OneTimeAddrEntry> verify(const std::string &addr);
        void del(const std::string &addr);

    private:
        std::string generate_tmpaddr(const email_addr &addr);
    };
}
