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

namespace ota
{
    std::optional<OneTimeAddr> onetimeaddr;
    std::unordered_set<std::string> rcpts;

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
                std::cerr << "Info: one time address " << rcpt << " was used" << std::endl;
                return SMFIS_CONTINUE;
            }
            else
            {
                smfi_setreply(ctx, "550", NULL, "Reject: Invalid One Time Address");
                std::cerr << "Warning: Invalied one time address " << rcpt << std::endl;
                return mlfi_cleanup(ctx, SMFIS_REJECT);
            }
        }
        else if (rcpts.contains(rcpt))
        {
            const std::string addr = onetimeaddr.value().create(rcpt);
            std::string addrrepr = addr;
            addrrepr = std::regex_replace(addrrepr, std::regex("@"), " [_at-mark_] ");
            addrrepr = std::regex_replace(addrrepr, std::regex("\\."), " [_dot_] ");

            std::cerr << "Info: new one time address " << addr << " is created" << std::endl;
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

    struct smfiDesc smfilter = {
        .xxfi_name = "OTA Filter",
        .xxfi_version = SMFI_VERSION,
        .xxfi_flags = SMFIF_ADDHDRS | SMFIF_ADDRCPT,
        .xxfi_connect = mlfi_connect,
        .xxfi_helo = NULL,
        .xxfi_envfrom = mlfi_envfrom,
        .xxfi_envrcpt = mlfi_envrcpt,
        .xxfi_header = NULL,
        .xxfi_eoh = NULL,
        .xxfi_body = NULL,
        .xxfi_eom = mlfi_eom,
        .xxfi_abort = mlfi_abort,
        .xxfi_close = NULL,
        .xxfi_unknown = NULL,
        .xxfi_data = NULL,
        .xxfi_negotiate = mlfi_negotiate,
    };
}
