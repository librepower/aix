# Web Services SRC Integration for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![License](https://img.shields.io/badge/license-MIT-green)

Part of the LibrePower web stack for AIX. This package integrates Apache httpd, nginx, and PHP-FPM with AIX System Resource Controller (SRC), enabling native service management with `startsrc`/`stopsrc`/`lssrc`.

Works with [php83](../php83/) and [mariadb11](../mariadb11/) to provide a complete LAMP/LEMP stack on AIX.

## Quick Start

```bash
# Install
dnf install librepower-web-src

# Start your web stack
startsrc -s httpd
startsrc -s php-fpm

# Check status
lssrc -s httpd
lssrc -s php-fpm
```

That's it. Services are registered automatically on install.

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

**[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

**[librepower.org](https://librepower.org)** — Launching February 2026

---

## Why SRC Integration?

AIX SRC provides unified service management:
- Start/stop services with `startsrc`/`stopsrc`
- Check status with `lssrc`
- Auto-start at boot via `/etc/inittab`
- Consistent management across all web services

## Installation

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install SRC integration
dnf install librepower-web-src
```

The package automatically registers httpd, nginx, and php-fpm with SRC on install.

## Service Commands

| Action | Command |
|--------|---------|
| Start Apache | `startsrc -s httpd` |
| Start nginx | `startsrc -s nginx` |
| Start PHP-FPM | `startsrc -s php-fpm` |
| Stop service | `stopsrc -s <service>` |
| Check status | `lssrc -s <service>` |

### Auto-Start at Boot

```bash
# Run the installer with boot option
/opt/freeware/libexec/librepower/web-src-install.sh boot
```

Or manually add to `/etc/inittab`:
```bash
mkitab 'httpd:2:once:/usr/bin/startsrc -s httpd'
mkitab 'php-fpm:2:once:/usr/bin/startsrc -s php-fpm'
```

### Manual Script Usage

```bash
# Register all services
/opt/freeware/libexec/librepower/web-src-install.sh

# Register individual services
/opt/freeware/libexec/librepower/web-src-install.sh httpd
/opt/freeware/libexec/librepower/web-src-install.sh nginx
/opt/freeware/libexec/librepower/web-src-install.sh php-fpm

# Check status
/opt/freeware/libexec/librepower/web-src-install.sh status

# Remove all SRC registrations
/opt/freeware/libexec/librepower/web-src-install.sh remove
```

## Technical Details

| Service | Binary | Foreground Flag |
|---------|--------|-----------------|
| Apache httpd | `/opt/freeware/sbin/httpd` | `-DFOREGROUND` |
| nginx | `/opt/freeware/sbin/nginx` | `-g 'daemon off;'` |
| PHP-FPM | `/opt/freeware/sbin/php-fpm` | `-F` |

**Note:** SRC requires services to run in foreground mode.

## Package Contents

```
web-src/
├── README.md
├── RPMS/
│   └── librepower-web-src-1.0-1.librepower.aix7.3.noarch.rpm
├── SPECS/
│   └── librepower-web-src.spec
└── scripts/
    └── web-src-install.sh
```

## Related Packages

- **php83** - PHP 8.3 for AIX
- **php83-fpm** - PHP-FPM daemon
- **mariadb11** - MariaDB database server (also SRC-integrated)

## License

MIT License - LibrePower

## Credits

Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source
