# One Time Address Milter

A postfix/sendmail milter for one time address to prevent spams.

## Architecture

1. Sender tries to send email to specific address (`me@example.com`).
2. SMTP server reject SMTP RCPT command and return message with a one time address (`tmp+abcdefghijk@example.com`).
3. Sender sends mail to the one time address again.
4. To header is modified (from one time address) to real recipient (`me@example.com`), and the mail will be sent.

## Build

- Requirements
  - g++ (>= C++17)
  - make
  - libmilter-dev

```
make
```

## Usage

- Requirements
  - libstdc++ (>= C++17)
  - libmilter

```shell
export OTA_MILTER_DOMAIN=example.com
export OTA_MILTER_RCPT=me@example.com
export OTA_MILTER_CONN=inet:6000

./main.out
```

postfix configuration

```conf
milter_default_action = accept
smtpd_milters = inet:127.0.0.1:6000
```

## Configuration

| Env Var                 | Description (Default)                            |
| ----------------------- | ------------------------------------------------ |
| `OTA_MILTER_DOMAIN`     | Domain for onetime address (Required)            |
| `OTA_MILTER_EXPIRES_IN` | Seconds to expires onetime address (`600`)       |
| `OTA_MILTER_RCPT`       | Space separated target recipient list (Required) |
| `OTA_MILTER_CONN`       | Milter connection argument (`inet:6000`)         |

## References

- https://milter-manager.osdn.jp/reference/ja/libmilter-mfapi.h.html
