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

namespace ota
{
    std::random_device seed_gen;
    std::mt19937 random_engine(seed_gen());

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
        OneTimeAddr(const OneTimeAddr &ota)
        {
            this->domain = ota.domain;
            this->expires_in = ota.expires_in;
            this->tmpaddrs = ota.tmpaddrs;
        }
        OneTimeAddr(const OneTimeAddr &&ota)
        {
            this->domain = std::move(ota.domain);
            this->expires_in = std::move(ota.expires_in);
            this->tmpaddrs = std::move(ota.tmpaddrs);
        }
        OneTimeAddr(const std::string &domain, const std::chrono::seconds expires_in);
        ~OneTimeAddr() = default;
        OneTimeAddr operator=(const OneTimeAddr &ota)
        {
            this->domain = ota.domain;
            this->expires_in = ota.expires_in;
            this->tmpaddrs = ota.tmpaddrs;
            return *this;
        }
        OneTimeAddr operator=(const OneTimeAddr &&ota)
        {
            this->domain = std::move(ota.domain);
            this->expires_in = std::move(ota.expires_in);
            this->tmpaddrs = std::move(ota.tmpaddrs);
            return std::move(*this);
        }

        std::string create(const std::string &realrcpt);
        std::optional<OneTimeAddrEntry> verify(const std::string &addr);
        void del(const std::string &addr);

    private:
        std::string generate_tmpaddr();
    };

    OneTimeAddr::OneTimeAddr(const std::string &domain, const std::chrono::seconds expires_in)
        : domain(domain), expires_in(expires_in)
    {
    }

    std::string OneTimeAddr::create(const std::string &realrcpt)
    {
        std::string addr;
        while (true)
        {
            addr = this->generate_tmpaddr();
            if (!this->tmpaddrs.contains(addr))
                break;
        }

        const auto &&expires_at = std::chrono::system_clock::now() + std::chrono::duration_cast<std::chrono::seconds>(this->expires_in);
        this->tmpaddrs[addr] = {expires_at, realrcpt};
        return addr;
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

    std::string OneTimeAddr::generate_tmpaddr()
    {
        const std::string chars = "abcdefghijklmnopqrstuvwxyz0123456789";
        std::ostringstream ss;
        ss << "tmp+";
        for (size_t i = 0; i < 10; i++)
        {
            std::uniform_int_distribution<int> dist(0, chars.size() - 1);
            int result = dist(random_engine);
            ss << chars[result];
        }
        ss << "@" << this->domain;
        return ss.str();
    }
}
