# PHP 8.3 - Modern PHP for AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![PHP](https://img.shields.io/badge/PHP-8.3.16-777BB4)
![License](https://img.shields.io/badge/license-PHP-green)

The latest PHP 8.3 LTS release, now available for AIX on IBM Power. Modern web development with 53 extensions including Fibers, opcache, and full database support.

![PHP 8.3 on AIX Demo](demo.gif)

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

**[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

**[librepower.org](https://librepower.org)** — Launching February 2026

---

## Why PHP 8.3 for AIX?

IBM AIX Toolbox provides PHP 7.4.33, which reached End-of-Life. This port brings modern PHP to AIX:

- **PHP 8.3 LTS** - Active support until December 2027
- **53 extensions** - Everything you need for web development
- **Fibers support** - Coroutines with native ppc64 assembly
- **PHP-FPM** - Production-ready process manager
- **Full database support** - MySQL, PostgreSQL, SQLite
- **Production ready** - Tested with WordPress, Nextcloud, Flarum, Lychee, Kanboard

### Performance on POWER9

Zend benchmark results:

| Test | Time |
|------|------|
| mandel | 0.416s |
| fibo(30) | 0.369s |
| ackermann(7) | 0.099s |
| **Total** | **2.436s** |

## Installation

### Option 1: dnf (Recommended)

Add the LibrePower repository and install with one command:

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install PHP CLI and extensions
dnf install php83

# Install PHP-FPM (optional)
dnf install php83-fpm

# Install development headers (optional)
dnf install php83-devel
```

Repository details: https://aix.librepower.org/

### Option 2: Direct RPM Download

```bash
cd /tmp

# Main package (CLI, CGI, 53 extensions)
curl -L -o php83-8.3.16-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/php83-8.3.16-1.librepower.aix7.3.ppc.rpm

# PHP-FPM (optional)
curl -L -o php83-fpm-8.3.16-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/php83-fpm-8.3.16-1.librepower.aix7.3.ppc.rpm

# Install
rpm -ivh php83-8.3.16-1.librepower.aix7.3.ppc.rpm
rpm -ivh php83-fpm-8.3.16-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

### Verify Installation

```bash
# Check version
/opt/freeware/bin/php -v
# PHP 8.3.16 (cli) (built: Jan 20 2026 22:15:48) (NTS)

# Count extensions
/opt/freeware/bin/php -m | wc -l
# 53

# Test key extensions
/opt/freeware/bin/php -r "echo 'bcmath: ' . bcadd('1.234', '5') . PHP_EOL;"
/opt/freeware/bin/php -r "echo 'mbstring: ' . mb_strlen('テスト') . ' chars' . PHP_EOL;"
/opt/freeware/bin/php -r "echo 'curl: ' . curl_version()['version'] . PHP_EOL;"
/opt/freeware/bin/php -r "echo 'openssl: ' . OPENSSL_VERSION_TEXT . PHP_EOL;"
```

### PHP-FPM

```bash
# Test configuration
/opt/freeware/sbin/php-fpm -t
# NOTICE: configuration file /opt/freeware/etc/php-fpm.conf test is successful

# Start PHP-FPM
/opt/freeware/sbin/php-fpm

# Default pool listens on 127.0.0.1:9000
```

### Hello World

```bash
echo '<?php echo "Hello from PHP " . PHP_VERSION . " on AIX!\n"; ?>' | php
# Hello from PHP 8.3.16 on AIX!
```

## Extensions (53)

All extensions are built-in and loaded by default:

| Category | Extensions |
|----------|------------|
| **Core** | Core, date, filter, hash, pcre, Reflection, SPL, standard |
| **Database** | mysqli, mysqlnd, PDO, pdo_mysql, pdo_pgsql, pdo_sqlite, pgsql, sqlite3 |
| **XML** | dom, libxml, SimpleXML, xml, xmlreader, xmlwriter, soap |
| **Crypto** | openssl, sodium |
| **Network** | curl, ftp, sockets |
| **Text** | mbstring, json, tokenizer, gettext |
| **Math** | bcmath, gmp |
| **Images** | gd, exif |
| **Compression** | bz2, zip, zlib |
| **System** | pcntl, posix, shmop, sysvmsg, sysvsem, sysvshm |
| **i18n** | intl |
| **Misc** | calendar, ctype, fileinfo, Phar, random, session, opcache |

### Extension Tests

| Extension | Test | Result |
|-----------|------|--------|
| bcmath | `bcadd('1.234', '5')` | 6 |
| mbstring | `mb_strlen('テスト')` | 3 chars |
| curl | `curl_version()` | 8.18.0 |
| openssl | `OPENSSL_VERSION_TEXT` | 3.0.13 |
| sodium | `SODIUM_LIBRARY_VERSION` | 1.0.19 |
| gd | `imagecreate(100,100)` | OK |
| intl | `NumberFormatter` | OK |
| Fibers | `Fiber::suspend()` | OK (ppc64 asm) |


## Recommended PHP Configuration

For web applications, add to `/opt/freeware/etc/php.ini`:

```ini
memory_limit = 128M
post_max_size = 16M
upload_max_filesize = 8M
max_execution_time = 300
max_input_time = 300
date.timezone = Your/Timezone
```

## AIX-Specific Notes

### Build Fixes Applied

This build required several AIX-specific fixes, documented for potential upstream contribution:

1. **64-bit host triplet**: `--host=powerpc64-ibm-aix7.3.4.0` for correct Fiber assembly detection
2. **OBJECT_MODE**: `export OBJECT_MODE=64` for AIX linker
3. **Archive tools**: `export AR="ar -X64"` and `export NM="nm -X64"`
4. **pthread linking**: `export LIBS="-lpthread"`

### Extensions Not Included

- **readline** - AIX readline 8.2 missing `rl_pending_input` symbol
- **iconv** - Conflicts with AIX native iconv implementation
- **ldap** - OpenLDAP linking issues on AIX 7.3
- **JIT** - Not supported on ppc64 architecture (expected)

### Upstream Contribution

All patches are documented and ready for submission to PHP project. See `BUILD.md` for details.

## Package Contents

```
php83/
├── demo.gif                                    # Demo animation
├── RPMS/
│   ├── php83-8.3.16-1.librepower.aix7.3.ppc.rpm
│   ├── php83-fpm-8.3.16-1.librepower.aix7.3.ppc.rpm
│   └── php83-devel-8.3.16-1.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── php83.spec
└── README.md
```

## Packages

| Package | Size | Contents |
|---------|------|----------|
| **php83** | 12 MB | CLI, CGI, opcache, 53 extensions |
| **php83-fpm** | 6 MB | PHP-FPM daemon |
| **php83-devel** | 1 MB | Headers, phpize, php-config |

## Installation Paths

```
/opt/freeware/bin/php              # PHP CLI (24.9 MB)
/opt/freeware/bin/php-cgi          # PHP CGI (24.7 MB)
/opt/freeware/sbin/php-fpm         # PHP-FPM (24.9 MB)
/opt/freeware/bin/phpize           # Extension build tool
/opt/freeware/bin/php-config       # Configuration tool
/opt/freeware/etc/php.ini          # Main configuration
/opt/freeware/etc/php-fpm.conf     # FPM configuration
/opt/freeware/etc/php-fpm.d/       # FPM pool configs
/opt/freeware/etc/php.d/           # Extension configs
/opt/freeware/include/php/         # Development headers
```

## Technical Details

| Component | Details |
|-----------|---------|
| **Platform** | AIX 7.3 TL4 |
| **Architecture** | POWER9 (ppc64, big-endian) |
| **Compiler** | GCC 13.3.0 |
| **Build flags** | `-maix64 -O2` |
| **Extensions** | 53 (all static) |
| **Benchmark** | 2.436s (Zend bench.php) |

## Requirements

- AIX 7.2+ (tested on 7.3 TL4)
- Dependencies from AIX Toolbox:
  - libxml2, openssl, zlib, curl
  - libpng, libjpeg-turbo, freetype
  - postgresql-libs, sqlite, gmp
  - bzip2, libzip, libsodium
  - icu, oniguruma

All dependencies available via `dnf` from AIX Toolbox.

## License

- PHP: [PHP License](https://www.php.net/license/)
- AIX packaging: PHP License (LibrePower)

## Credits

- PHP by [The PHP Group](https://www.php.net)
- AIX port and packaging by [LibrePower](https://librepower.org)

## Support

- **Documentation**: https://gitlab.com/librepower/aix
- **Repository**: https://aix.librepower.org
- **Newsletter**: https://librepower.substack.com
- **Email**: hello@librepower.org

---

## SRC Integration (AIX System Resource Controller)

Manage PHP-FPM and web services with native AIX SRC commands.

### Easy Setup (Recommended)

```bash
# Install SRC integration package - auto-registers httpd, nginx, php-fpm
dnf install librepower-web-src

# Start services (LAMP with Apache)
startsrc -s httpd
startsrc -s php-fpm

# Or LEMP with nginx
startsrc -s nginx
startsrc -s php-fpm

# Check status
lssrc -s httpd      # or nginx
lssrc -s php-fpm

# Enable auto-start at boot
/opt/freeware/libexec/librepower/web-src-install.sh boot
```

See [web-src](../web-src/) for full documentation.

### Manual Registration (Advanced)

If you prefer to register services manually:

```bash
# PHP-FPM
mkssys -s php-fpm -p /opt/freeware/sbin/php-fpm -a "-F" -u 0 -S -n 15 -f 9

# Apache httpd
mkssys -s httpd -p /opt/freeware/sbin/httpd -a "-DFOREGROUND" -u 0 -S -n 15 -f 9

# nginx
mkssys -s nginx -p /opt/freeware/sbin/nginx -a "-g 'daemon off;'" -u 0 -S -n 15 -f 9
```

**Note**: SRC requires foreground mode flags (`-F` for php-fpm, `-DFOREGROUND` for httpd, `-g 'daemon off;'` for nginx).

---

## Complete Web Stack (LAMP/LEMP)

PHP 8.3 integrates with IBM AIX Toolbox for a complete stack:

| Component | Version | Source | Install |
|-----------|---------|--------|---------|
| Apache httpd | 2.4.66 | IBM AIX Toolbox | `dnf install httpd` |
| nginx | 1.27.4 | IBM AIX Toolbox | `dnf install nginx` |
| **PHP** | **8.3.16** | **LibrePower** | `dnf install php83 php83-fpm` |
| **MariaDB** | **11.8.0** | **LibrePower** | `dnf install mariadb11` |
| PostgreSQL | 16.x | IBM AIX Toolbox | `dnf install postgresql16-server` |

### Apache + PHP-FPM Configuration

Add to `/opt/freeware/etc/httpd/conf/httpd.conf`:

```apache
# Load modules (use 32-bit path - Apache from Toolbox is 32-bit)
LoadModule proxy_module /opt/freeware/lib/httpd/modules/mod_proxy.so
LoadModule proxy_fcgi_module /opt/freeware/lib/httpd/modules/mod_proxy_fcgi.so
LoadModule rewrite_module /opt/freeware/lib/httpd/modules/mod_rewrite.so

# PHP-FPM handler
<FilesMatch \.php$>
    SetHandler "proxy:fcgi://127.0.0.1:9000"
</FilesMatch>

# Enable .htaccess for WordPress, Laravel, etc.
<Directory "/var/www/htdocs">
    AllowOverride All
</Directory>

# Timeout for large operations
ProxyTimeout 300

DirectoryIndex index.php index.html
```

### PHP Configuration for Web Apps

Add to `/opt/freeware/etc/php.ini`:

```ini
pdo_mysql.default_socket=/opt/freeware/var/lib/mysql/mysql.sock
mysqli.default_socket=/opt/freeware/var/lib/mysql/mysql.sock
max_execution_time = 300
```

---

## Tested Applications

Validated PHP applications on AIX 7.3 with this stack:

| Application | Type | Database | Status |
|-------------|------|----------|--------|
| **WordPress 6.9** | CMS | MariaDB | ✅ Working |
| **Kanboard** | Project Management | SQLite | ✅ Working |
| **Lychee 5.5** | Photo Gallery | MariaDB | ✅ Working |
| **Flarum 2.0** | Forum | MariaDB | ✅ Working |
| **Nextcloud** | Cloud Storage | MariaDB | ✅ Working |

### WordPress Plugins Tested

- WooCommerce ✅
- Duplicator ✅
- Yoast SEO ✅

### Developer Tools

```bash
# WP-CLI
curl -O https://raw.githubusercontent.com/wp-cli/builds/gh-pages/phar/wp-cli.phar
mv wp-cli.phar /opt/freeware/bin/wp-cli.phar
echo '#!/bin/bash
exec /opt/freeware/bin/php /opt/freeware/bin/wp-cli.phar "$@"' > /opt/freeware/bin/wp
chmod +x /opt/freeware/bin/wp

# Composer
curl -sS https://getcomposer.org/installer | /opt/freeware/bin/php
mv composer.phar /opt/freeware/bin/composer
```

---

## Dependencies (All Available)

All dependencies available via IBM AIX Toolbox - no additional porting required:

| Package | Version |
|---------|---------|
| OpenSSL | 3.0.16 |
| CA Certificates | 301 certs |
| zlib | 1.3.1 |
| libnghttp2 | 1.62.1 |
| pcre2 | 10.45 |
| curl | 8.18.0 |
| libxml2 | 2.14.6 |
| ImageMagick | 7.1.2.11 |
