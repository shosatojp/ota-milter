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
#include <sysexits.h>

#include <libmilter/mfapi.h>

#include "utils.hpp"
#include "ota.hpp"
#include "mlfi.hpp"

using ota::onetimeaddr;
using ota::OneTimeAddr;
using ota::rcpts;
using ota::smfilter;

int main(int argc, const char *argv[])
{
    /* setup domain */
    const char *domain_ptr = getenv("OTA_MILTER_DOMAIN");
    if (!domain_ptr)
    {
        std::cerr << "OTA_MILTER_DOMAIN is required" << std::endl;
        exit(EX_CONFIG);
    }
    const std::string domain = domain_ptr;
    std::cerr << "Info: domain: " << domain << std::endl;

    /* setup expires_in */
    std::chrono::seconds expires_in = std::chrono::seconds(600);
    const char *expires_in_ptr = getenv("OTA_MILTER_EXPIRES_IN");
    if (expires_in_ptr)
    {
        expires_in = std::chrono::seconds(std::atoi(expires_in_ptr));
    }
    std::cerr << "Info: expires_in: " << expires_in.count() << " seconds" << std::endl;

    /* setup onetimeaddr */
    onetimeaddr = std::make_unique<OneTimeAddr>(domain, expires_in);
    std::cerr << "OTA_MILTER_RCPT" << std::endl;

    /* setup rcpts */
    const char *rcpts_ptr = getenv("OTA_MILTER_RCPT");
    if (!rcpts_ptr)
    {
        std::cerr << "OTA_MILTER_RCPT is required" << std::endl;
        exit(EX_CONFIG);
    }
    for (auto &&_rcpt : ota::utils::str_split(rcpts_ptr, " "))
    {
        const std::string rcpt = ota::utils::str_trim(_rcpt);
        rcpts.insert(rcpt);
        std::cerr << "Info: target recipient: " << rcpt << std::endl;
    }

    /* start milter server */
    const char *conn_ptr = getenv("OTA_MILTER_CONN");
    if (!conn_ptr)
    {
        conn_ptr = "inet:6000";
    }

    if (smfi_setconn(const_cast<char *>(conn_ptr)) == MI_FAILURE)
    {
        std::cerr << "smfi_setconn failed" << std::endl;
        exit(EX_SOFTWARE);
    }
    else
    {
        std::cerr << "Info: smfi_setconn: " << conn_ptr << std::endl;
    }
    if (smfi_register(smfilter) == MI_FAILURE)
    {
        std::cerr << "smfi_register failed" << std::endl;
        exit(EX_UNAVAILABLE);
    }
    return smfi_main();
}
