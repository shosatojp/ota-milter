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

namespace ota
{
    struct OneTimeAddrEntry
    {
        std::chrono::time_point<std::chrono::system_clock> expires_at;
        std::string realrcpt;
    };

    class OneTimeAddr
    {
    private:
        std::unordered_map<std::string, OneTimeAddrEntry> tmpaddrs;

    public:
        std::string domain;
        std::chrono::seconds expires_in;
        std::thread cleanup_thread;
        OneTimeAddr(const OneTimeAddr &ota);
        OneTimeAddr(const OneTimeAddr &&ota);
        OneTimeAddr(const std::string &domain, const std::chrono::seconds expires_in);
        ~OneTimeAddr();
        OneTimeAddr operator=(const OneTimeAddr &ota);
        OneTimeAddr operator=(const OneTimeAddr &&ota);
        std::string create(const std::string &realrcpt);
        std::optional<OneTimeAddrEntry> verify(const std::string &addr);
        void del(const std::string &addr);

    private:
        std::string generate_tmpaddr();
    };
}
