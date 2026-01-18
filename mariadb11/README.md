# MariaDB 11.8.0 - Database Server for AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

![AIX 7.3](https://img.shields.io/badge/AIX-7.2+-blue)
![MariaDB](https://img.shields.io/badge/MariaDB-11.8.0-orange)
![License](https://img.shields.io/badge/license-GPLv2-green)

Community-developed fork of MySQL, now available for AIX on IBM Power. Full SQL database server with Performance Schema support and native AIX SRC integration.

![MariaDB 11.8 on AIX Demo](demo.gif)

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

**[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

**[librepower.org](https://librepower.org)** — Launching February 2026

---

## What is MariaDB?

MariaDB is a high-performance, open-source relational database server. It's a drop-in replacement for MySQL with additional features and optimizations.

**Why use MariaDB on AIX:**
- Enterprise-grade SQL database on POWER architecture
- Native AIX SRC (System Resource Controller) integration
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

Repository details: https://aix.librepower.org/

> **Note**: Package named `mariadb11` to avoid conflicts with AIX Toolbox's `mariadb10.11`. Both can coexist on the same system.

### Option 2: Direct RPM Download

```bash
cd /tmp

curl -L -o mariadb11-11.8.0-4.librepower.ppc.rpm \
  https://aix.librepower.org/packages/mariadb11-11.8.0-4.librepower.ppc.rpm

rpm -ivh mariadb11-11.8.0-4.librepower.ppc.rpm
```

## Quick Start

### Initialize Database

```bash
cd /opt/freeware/mariadb

# Initialize system tables (first time only)
./bin/mysql_install_db \
  --basedir=/opt/freeware/mariadb \
  --datadir=/var/mariadb/data \
  --user=mysql

# Verify initialization
ls -la /var/mariadb/data/
```

### Start Server with AIX SRC (Recommended)

MariaDB integrates with AIX System Resource Controller for standard service management:

```bash
# Start MariaDB
startsrc -s mariadb11

# Check status
lssrc -s mariadb11
# Output: mariadb11    33030542     active

# Stop MariaDB
stopsrc -s mariadb11
```

**Enable auto-start on boot:**

```bash
# Add to inittab
mkitab "mariadb11:2:once:/usr/bin/startsrc -s mariadb11 > /dev/console 2>&1"

# Remove from inittab (if needed)
rmitab mariadb11
```

**Benefits of SRC integration:**
- Standard AIX service management (same as sshd, inetd, etc.)
- Automatic LIBPATH configuration
- Clean startup/shutdown with SIGTERM
- Process tracking and monitoring

### Alternative: Manual Start

```bash
cd /opt/freeware/mariadb

# Start directly
./bin/mariadbd \
  --basedir=/opt/freeware/mariadb \
  --datadir=/var/mariadb/data \
  --user=mysql &

# Or use mysqld_safe for automatic restart
./bin/mysqld_safe \
  --basedir=/opt/freeware/mariadb \
  --datadir=/var/mariadb/data \
  --user=mysql &
```

### Connect and Test

```bash
# Connect as root (no password initially)
mariadb -u root

# Or use mysql client
mysql -u root

# In MariaDB prompt:
MariaDB> SELECT VERSION();
+------------------+
| VERSION()        |
+------------------+
| 11.8.0-MariaDB   |
+------------------+

MariaDB> SHOW DATABASES;
MariaDB> CREATE DATABASE testdb;
MariaDB> USE testdb;
MariaDB> CREATE TABLE servers (id INT, model VARCHAR(50), arch VARCHAR(20));
MariaDB> INSERT INTO servers VALUES (1, 'Power S924', 'POWER9');
MariaDB> SELECT * FROM servers;
MariaDB> EXIT;
```

### Verify Installation

```bash
# Check service status
lssrc -s mariadb11

# Verify process
ps -ef | grep mariadbd

# Check network port
netstat -an | grep 3306

# Test connection
echo "SELECT VERSION();" | mariadb -u root
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
- **1247 performance instruments** available

```bash
MariaDB> SELECT COUNT(*) FROM performance_schema.setup_instruments;
+----------+
| COUNT(*) |
+----------+
|     1247 |
+----------+
```

### Storage Engines

All major storage engines are fully functional:

| Engine | Support | Description |
|--------|---------|-------------|
| InnoDB | DEFAULT | Transactions, row-level locking, ACID |
| Aria | YES | Crash-safe tables with MyISAM heritage |
| MEMORY | YES | Hash based, stored in memory |
| MyISAM | YES | Traditional MySQL storage engine |

## Package Contents

```
mariadb11/
├── demo.gif                              # Demo GIF
├── RPMS/
│   └── mariadb11-11.8.0-4.librepower.ppc.rpm
├── SPECS/
│   └── mariadb.spec
├── SOURCES/
│   └── mariadb-aix-perfschema.patch
└── README.md
```

## What's Included

- **mariadbd** - Main database server (via libserver.so)
- **mysql_install_db** - Database initialization utility
- **mysqld_safe** - Server startup wrapper with auto-restart
- **my_print_defaults** - Configuration display utility
- **resolveip** - IP/hostname resolver
- **perror** - Error code lookup
- **Performance Schema** - Performance monitoring tables
- **System tables and data** - Complete MariaDB installation
- **SRC integration** - Native AIX service management

## Technical Details

### Build Information

| Component | Details |
|-----------|---------|
| **Platform** | AIX 7.3 TL4 |
| **Architecture** | POWER (tested on POWER9) |
| **Compiler** | GCC 13.3.0 |
| **CMake** | 4.2.0 |
| **Binary Size** | 703 MB (libserver.so) |
| **Build Time** | ~15 minutes (-j96) |
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

## Installation Paths

```
/opt/freeware/mariadb/bin/         # Server binaries
/opt/freeware/mariadb/lib/         # libserver.so (703MB)
/opt/freeware/mariadb/share/       # Error messages, configs
/var/mariadb/data/                 # Database files
```

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
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source

## Support

- **Documentation**: https://gitlab.com/librepower/aix
- **Repository**: https://aix.librepower.org
- **Newsletter**: https://librepower.substack.com
- **Email**: hello@librepower.org
