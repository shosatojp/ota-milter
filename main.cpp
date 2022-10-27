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

using ota::OneTimeAddr;

std::optional<OneTimeAddr> onetimeaddr;
std::unordered_set<std::string> rcpts;

struct mlfiPriv
{
    std::vector<std::string> rcpts;
};

sfsistat mlfi_cleanup(SMFICTX *ctx, sfsistat stat)
{
    struct mlfiPriv *priv = (struct mlfiPriv *)smfi_getpriv(ctx);
    if (priv != nullptr)
    {
        delete priv;
        smfi_setpriv(ctx, nullptr);
    }
    return stat;
}

sfsistat mlfi_connect(SMFICTX *ctx, char *hostname, struct sockaddr *hostaddr)
{
    struct mlfiPriv *priv = (struct mlfiPriv *)calloc(1, sizeof(struct mlfiPriv));
    if (priv == NULL)
    {
        return SMFIS_TEMPFAIL;
    }

    smfi_setpriv(ctx, priv);

    return SMFIS_CONTINUE;
}

sfsistat mlfi_envfrom(SMFICTX *ctx, char **argv)
{
    return SMFIS_CONTINUE;
}

sfsistat mlfi_envrcpt(SMFICTX *ctx, char **argv)
{
    struct mlfiPriv *priv = (struct mlfiPriv *)smfi_getpriv(ctx);
    char *rcptaddr = smfi_getsymval(ctx, "{rcpt_addr}");
    if (rcptaddr == nullptr)
    {
        return mlfi_cleanup(ctx, SMFIS_TEMPFAIL);
    }
    const std::string rcpt = rcptaddr;

    if (rcpt.starts_with("tmp+"))
    {
        const auto &&entry = onetimeaddr.value().verify(rcpt);
        if (entry.has_value())
        {
            priv->rcpts.push_back(entry.value().realrcpt);
            onetimeaddr.value().del(rcpt);
            return SMFIS_CONTINUE;
        }
        else
        {
            smfi_setreply(ctx, "500", NULL, "Reject: Invalid One Time Address");
            return mlfi_cleanup(ctx, SMFIS_REJECT);
        }
    }
    else if (rcpts.contains(rcpt))
    {
        std::string addr = onetimeaddr.value().create(rcpt);
        std::string addrrepr = addr;
        addrrepr = std::regex_replace(addrrepr, std::regex("@"), " [_at-mark_] ");
        addrrepr = std::regex_replace(addrrepr, std::regex("\\."), " [_dot_] ");

        std::cout << "new one time address " << addr << " is created" << std::endl;
        std::ostringstream ss;
        ss << "Reject: This email address is send only. Please send again to \""
           << addrrepr << "\" in " << onetimeaddr.value().expires_in.count() << " seconds.";
        const auto ss_str = ss.str();
        smfi_setreply(ctx, "500", NULL, const_cast<char *>(ss_str.c_str()));
        return mlfi_cleanup(ctx, SMFIS_REJECT);
    }
    else
    {
        priv->rcpts.push_back(rcpt);
    }
    return SMFIS_CONTINUE;
}

sfsistat mlfi_eom(SMFICTX *ctx)
{
    struct mlfiPriv *priv = (struct mlfiPriv *)smfi_getpriv(ctx);
    const std::string rcpts_str = ota::utils::str_join(priv->rcpts, ", ");
    if (smfi_chgheader(ctx, "To", 1, const_cast<char *>(rcpts_str.c_str())) == MI_FAILURE)
    {
        std::cerr << "Error: failed to smfi_chgheader to " << rcpts_str << std::endl;
    }

    return mlfi_cleanup(ctx, SMFIS_CONTINUE);
}

sfsistat mlfi_abort(SMFICTX *ctx)
{
    return mlfi_cleanup(ctx, SMFIS_TEMPFAIL);
}

// https://www.ibm.com/docs/ja/aix/7.1?topic=functions-xxfi-negotiate-callback-function
sfsistat mlfi_negotiate(
    SMFICTX *ctx,
    unsigned long f0, unsigned long f1, unsigned long f2, unsigned long f3,
    unsigned long *pf0, unsigned long *pf1, unsigned long *pf2, unsigned long *pf3)
{
    return SMFIS_ALL_OPTS;
}

struct smfiDesc smfilter =
    {
        .xxfi_name = "OTA Filter",    /* filter name */
        .xxfi_version = SMFI_VERSION, /* version code -- do not change */
        .xxfi_flags = SMFIF_ADDHDRS | SMFIF_ADDRCPT,
        /* flags */
        .xxfi_connect = mlfi_connect,    /* connection info filter */
        .xxfi_helo = NULL,               /* SMTP HELO command filter */
        .xxfi_envfrom = mlfi_envfrom,    /* envelope sender filter */
        .xxfi_envrcpt = mlfi_envrcpt,    /* envelope recipient filter */
        .xxfi_header = NULL,             /* header filter */
        .xxfi_eoh = NULL,                /* end of header */
        .xxfi_body = NULL,               /* body block filter */
        .xxfi_eom = mlfi_eom,            /* end of message */
        .xxfi_abort = mlfi_abort,        /* message aborted */
        .xxfi_close = NULL,              /* connection cleanup */
        .xxfi_unknown = NULL,            /* unknown SMTP commands */
        .xxfi_data = NULL,               /* DATA command */
        .xxfi_negotiate = mlfi_negotiate /* Once, at the start of each SMTP connection */
};

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
    onetimeaddr = OneTimeAddr{domain, expires_in};

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
