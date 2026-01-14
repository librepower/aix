# MariaDB 11.8.0 - Database Server for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.2+-blue)
![MariaDB](https://img.shields.io/badge/MariaDB-11.8.0-orange)
![License](https://img.shields.io/badge/license-GPLv2-green)

Community-developed fork of MySQL, now available for AIX on IBM Power. Full SQL database server with Performance Schema support.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## What is MariaDB?

MariaDB is a high-performance, open-source relational database server. It's a drop-in replacement for MySQL with additional features and optimizations.

**Why use MariaDB on AIX:**
- Enterprise-grade SQL database on POWER architecture
- High availability and clustering support
- Compatible with MySQL applications and tools
- Active development and community support
- No Oracle licensing concerns

## Installation

### Option 1: dnf (Recommended)

Add the LibrePower repository and install with one command:

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install mariadb11
```

📦 Repository details: https://aix.librepower.org/

> **Note**: Package named `mariadb11` to avoid conflicts with AIX Toolbox's `mariadb10.11`. Both can coexist on the same system.

### Option 2: Direct RPM Download

```bash
cd /tmp

curl -L -o mariadb11-11.8.0-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/mariadb11-11.8.0-1.librepower.aix7.3.ppc.rpm

rpm -ivh mariadb11-11.8.0-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

### Initialize Database

```bash
cd /opt/freeware/mariadb

# Create data directory
mkdir -p data
chown mysql:staff data
chmod 750 data

# Initialize system tables
./bin/mysql_install_db \
  --basedir=/opt/freeware/mariadb \
  --datadir=/opt/freeware/mariadb/data \
  --user=mysql
```

### Start Server

```bash
# Start server (foreground)
./bin/mariadbd \
  --basedir=/opt/freeware/mariadb \
  --datadir=/opt/freeware/mariadb/data \
  --user=mysql

# Or use mysqld_safe for automatic restart
./bin/mysqld_safe \
  --basedir=/opt/freeware/mariadb \
  --datadir=/opt/freeware/mariadb/data \
  --user=mysql &
```

### Connect and Test

```bash
# Connect as root (no password initially)
mysql -u root

# In MySQL prompt:
MariaDB> SHOW DATABASES;
MariaDB> CREATE DATABASE testdb;
MariaDB> USE testdb;
MariaDB> CREATE TABLE test (id INT, name VARCHAR(50));
MariaDB> INSERT INTO test VALUES (1, 'AIX on POWER');
MariaDB> SELECT * FROM test;
MariaDB> EXIT;
```

### Secure Installation

```bash
# Set root password and secure defaults
./bin/mysql_secure_installation
```

## AIX-Specific Notes

### Threading Library

The RPM package includes a wrapper script that automatically sets the correct `LIBPATH` to use pthread-enabled libstdc++. No manual configuration needed!

**Behind the scenes:**
```bash
export LIBPATH=/opt/freeware/lib/pthread:\
/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:\
/opt/freeware/lib:\
/usr/lib
```

This ensures C++11 threading features (`std::thread`, `std::condition_variable`, `std::mutex`) work correctly.

### Performance Schema

Fully working with AIX-specific patches:
- Fixed incorrect detection of macOS-specific `pthread_threadid_np()`
- Fixed incorrect detection of OpenBSD-specific `getthrid()`
- Uses POSIX-standard `pthread_self()` fallback

## Package Contents

```
mariadb11/
├── RPMS/
│   └── mariadb11-11.8.0-1.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── mariadb.spec
├── SOURCES/
│   └── mariadb-aix-perfschema.patch
└── README.md
```

## What's Included

- **mariadbd** - Main database server
- **mysql_install_db** - Database initialization utility
- **mysqld_safe** - Server startup wrapper with auto-restart
- **my_print_defaults** - Configuration display utility
- **resolveip** - IP/hostname resolver
- **perror** - Error code lookup
- **Performance Schema** - Performance monitoring tables
- **System tables and data** - Complete MariaDB installation

## Technical Details

### Build Information

| Component | Details |
|-----------|---------|
| **Platform** | AIX 7.3 TL5 SP2 |
| **Architecture** | POWER (tested on POWER9) |
| **Compiler** | GCC 13.3.0 |
| **CMake** | 4.2.0 |
| **Binary Size** | 703 MB (libserver.so) |
| **Patches** | 2 CMake configuration changes |

### Patches Applied

1. **pthread_threadid_np Detection**: Skip macOS-specific function detection on AIX
2. **getthrid Detection**: Skip OpenBSD-specific function detection on AIX

**Status**: Patches ready for upstream submission to MariaDB project.

### Why These Patches Work

These are **minimal, non-invasive changes**:
- Only modify CMake platform detection logic
- No code changes to MariaDB itself
- Use existing POSIX fallback implementations
- Ideal candidates for upstream inclusion

## Complete Documentation

For detailed technical information, see:

📚 **[gitlab.com/librepower/mariadb](https://gitlab.com/librepower/mariadb)**

Includes:
- **PATCHES.md** - Complete technical analysis of patches
- **BUGS_REPORT.md** - Detailed bug reports for upstream
- **UPSTREAM_PR_GUIDE.md** - Guide for submitting to MariaDB
- **mariadb-aix-perfschema.patch** - Unified patch file

## Requirements

- AIX 7.2+ (tested on 7.3)
- GCC runtime libraries (from AIX Toolbox)
- ~1 GB disk space for installation
- ~500 MB free RAM minimum

## Upstream Contribution

These patches are ready for submission to the MariaDB project. They benefit the entire AIX community by enabling MariaDB without source modifications.

**Want to help?** The patches are documented and ready for submission. Contact us if you'd like to contribute to getting AIX support into upstream MariaDB.

## License

- MariaDB: GPLv2 (MariaDB Foundation)
- AIX patches and packaging: GPLv2 (LibrePower)

## Credits

- MariaDB by [MariaDB Foundation](https://mariadb.org)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍

## Support

- **Documentation**: https://gitlab.com/librepower/mariadb
- **Repository**: https://aix.librepower.org
- **Newsletter**: https://librepower.substack.com
- **Email**: hello@librepower.org
