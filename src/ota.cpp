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

#include "ota.hpp"
#include "utils.hpp"

namespace ota
{
    OneTimeAddr::OneTimeAddr(const std::vector<email_addr> &_rcpts, const std::chrono::seconds _expires_in)
        : random_engine(seed_gen()), rcpts(_rcpts), expires_in(_expires_in)
    {
        this->cleanup_thread = std::thread(
            [this]
            {
                std::cerr << "Info: cleanup thread started" << std::endl;
                while (true)
                {
                    const auto now = std::chrono::system_clock::now();
                    std::unordered_set<std::string> deletion_list;
                    for (auto &&[tmpaddr, entry] : this->tmpaddrs)
                    {
                        if (entry.expires_at < now)
                        {
                            deletion_list.insert(std::string(tmpaddr));
                        }
                    }
                    for (auto &&tmpaddr : deletion_list)
                    {
                        this->tmpaddrs.erase(tmpaddr);
                    }
                    if (deletion_list.size() > 0)
                    {
                        std::cerr << "Info: cleaned up " << deletion_list.size() << " addrs" << std::endl;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            });
    }

    OneTimeAddr::~OneTimeAddr()
    {
        this->cleanup_thread.join();
    }

    std::string OneTimeAddr::create(const email_addr &realrcpt)
    {
        std::string addr;
        while (true)
        {
            addr = this->generate_tmpaddr(realrcpt);
            if (!this->tmpaddrs.contains(addr))
                break;
        }

        const auto &&expires_at = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(this->expires_in);
        this->tmpaddrs[addr] = {expires_at, realrcpt};
        return addr;
    }

    std::optional<email_addr> OneTimeAddr::match(const std::string &addr)
    {
        const auto parsed = ota::utils::parse_email_addr(addr);
        if (!parsed.has_value())
            return std::nullopt;

        for (auto &&rcpt : this->rcpts)
        {
            if (parsed.value().domain == rcpt.domain && parsed.value().user == rcpt.user)
            {
                return parsed.value();
            }
        }

        return std::nullopt;
    }

    std::optional<OneTimeAddrEntry> OneTimeAddr::verify(const std::string &addr)
    {
        if (!this->tmpaddrs.contains(addr))
        {
            return std::nullopt;
        }

        const OneTimeAddrEntry &entry = this->tmpaddrs[addr];
        if (entry.expires_at < std::chrono::system_clock::now())
        {
            return std::nullopt;
        }

        return entry;
    }

    void OneTimeAddr::del(const std::string &addr)
    {
        if (this->tmpaddrs.contains(addr))
        {
            this->tmpaddrs.erase(addr);
        }
    }

    std::string OneTimeAddr::generate_tmpaddr(const email_addr &addr)
    {
        const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
        std::ostringstream ss;
        ss << addr.user << '+';
        for (size_t i = 0; i < 10; i++)
        {
            std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);
            size_t result = dist(random_engine);
            ss << chars[result];
        }
        ss << "@" << addr.domain;
        return ss.str();
    }
}
