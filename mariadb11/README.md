# MariaDB 11.8.5 LTS - Database Server for AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

![AIX 7.3](https://img.shields.io/badge/AIX-7.2+-blue)
![MariaDB](https://img.shields.io/badge/MariaDB-11.8.5_LTS-orange)
![Thread Pool](https://img.shields.io/badge/Thread_Pool-pool--of--threads-green)
![License](https://img.shields.io/badge/license-GPLv2-green)

Community-developed fork of MySQL, now available for AIX on IBM Power. Full SQL database server with **native thread pool** (`pool-of-threads`) and Performance Schema support.

![MariaDB 11.8 on AIX Demo](demo.gif)

## Join the Community

LibrePower is more than AIX -- we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

- **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.
- **[librepower.org](https://librepower.org)** -- Launching February 2026

---

## What's New in 11.8.5

### Thread Pool (pool-of-threads)

MariaDB on AIX now supports the **pool-of-threads** thread handling model, previously unavailable on AIX. This is the same high-performance model used on Linux, FreeBSD, Solaris, and Windows.

**Performance improvement** (mysqlslap mixed workload):

| Clients | one-thread-per-connection | pool-of-threads | Improvement |
|---------|--------------------------|-----------------|-------------|
| 50      | 5.437s                   | 1.917s          | **65% faster** |
| 100     | 11.340s                  | 1.964s          | **83% faster** |

The thread pool is enabled by default in this package. No configuration needed.

### Optimized Build

Built with `-O3 -mcpu=power9 -mtune=power9` (Release mode) for maximum performance on POWER9+ systems.

### QA Validated

- 1,000 concurrent clients: PASS
- 30 min sustained load (100 clients, 11.15M queries): 0 errors
- Memory stable (zero drift across 29 checkpoints)
- MTR: 709 pass, 50 fail (federated/platform, none thread-pool specific)

---

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install mariadb11
```

Repository details: https://aix.librepower.org/

> **Note**: Package named `mariadb11` to avoid conflicts with AIX Toolbox's `mariadb10.11`. Both can coexist.

### Option 2: Direct RPM Download

```bash
curl -L -o mariadb11-11.8.5-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/mariadb11-11.8.5-1.librepower.aix7.3.ppc.rpm

rpm -ivh mariadb11-11.8.5-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

### Initialize Database

```bash
cd /opt/freeware/mariadb

# Create data directory
mkdir -p /var/mariadb/data
chown mysql:mysql /var/mariadb/data
chmod 750 /var/mariadb/data

# Initialize system tables
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

The SRC service starts MariaDB with pool-of-threads enabled and optimized defaults.

### Connect and Test

```bash
# Connect as root (no password initially)
/opt/freeware/mariadb/bin/mariadb -u root -S /tmp/mysql.sock

# In MariaDB prompt:
MariaDB> SELECT VERSION();
+-------------------+
| VERSION()         |
+-------------------+
| 11.8.5-MariaDB    |
+-------------------+

MariaDB> SHOW VARIABLES LIKE 'thread_handling';
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
./bin/mysql_secure_installation
```

## Recommended Configuration

For production use, add to your configuration file:

```ini
[mariadbd]
thread_handling            = pool-of-threads
thread_pool_size           = 12
thread_stack               = 512K
innodb_buffer_pool_size    = 1G
innodb_adaptive_hash_index = ON
max_connections            = 2000
```

> **Note**: `thread-stack=512K` is required when using `-O3` optimized builds. The aggressive inlining increases stack usage beyond the default.

## AIX-Specific Notes

### Threading Library

The RPM package includes a wrapper script that sets the correct `LIBPATH` for pthread-enabled libstdc++. No manual configuration needed.

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

### AIX SRC Integration

The package registers a System Resource Controller (SRC) subsystem `mariadb11` with optimized defaults for pool-of-threads operation.

## Package Contents

```
mariadb11/
├── demo.gif                              # Demo GIF
├── RPMS/
│   └── mariadb11-11.8.5-1.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── mariadb11.spec
├── SOURCES/
│   ├── mariadb-aix-perfschema.patch     # CMake bug fixes
│   └── threadpool_aix_pollset.patch     # Thread pool implementation
└── README.md
```

## What's Included

- **mariadbd** - Main database server with pool-of-threads support
- **mariadb** - Command-line client
- **mariadb-admin** - Server administration tool
- **mariadb-dump** - Logical backup utility
- **mariadb-import** - Data import tool
- **mariadb-slap** - Load emulation client
- **mysql_install_db** - Database initialization
- **mysqld_safe** - Server startup wrapper with auto-restart
- **my_print_defaults** - Configuration display utility
- **resolveip** - IP/hostname resolver
- **perror** - Error code lookup
- **Performance Schema** - Performance monitoring (1247 instruments)
- **SRC integration** - Native AIX service management

## Technical Details

### Build Information

| Component | Details |
|-----------|---------|
| **Version** | 11.8.5 LTS |
| **Platform** | AIX 7.3 TL4 |
| **Architecture** | POWER (tested on POWER9) |
| **Compiler** | GCC 13.3.0 |
| **CMake** | 4.2.0 |
| **Optimization** | `-O3 -mcpu=power9 -mtune=power9` |
| **Thread Handling** | pool-of-threads (AIX pollset) |
| **Binary Size** | ~60 MB (libserver.so) |
| **Patches** | 3 (2 CMake fixes + 1 thread pool) |

### Patches Applied

1. **pthread_threadid_np Detection**: Skip macOS-specific function detection on AIX
2. **getthrid Detection**: Skip OpenBSD-specific function detection on AIX
3. **Thread Pool (pollset)**: Native AIX pollset support with ONESHOT simulation and per-pollset mutex for concurrent safety

**Status**: Patches submitted to MariaDB upstream (JIRA).

### Thread Pool Implementation

The thread pool uses AIX `pollset(2)` as the I/O multiplexing backend, with two key innovations:

1. **ONESHOT simulation**: Fds are removed from pollset after events fire (`PS_DELETE`) and re-added when ready (`PS_ADD`), preventing duplicate event delivery.

2. **Per-pollset mutex**: Serializes concurrent `pollset_poll_ext` calls between the listener thread (blocking) and worker threads (non-blocking via `trylock`).

This matches the behavior of Linux `epoll` with `EPOLLONESHOT`.

## Installation Paths

```
/opt/freeware/mariadb/bin/         # Server binaries
/opt/freeware/mariadb/lib/         # libserver.so (~60MB)
/opt/freeware/mariadb/share/       # Error messages, configs
/var/mariadb/data/                 # Database files
```

## Requirements

- AIX 7.2+ (tested on 7.3 TL4)
- GCC runtime libraries (from AIX Toolbox)
- ~1 GB disk space
- ~500 MB free RAM minimum

## Upstream Contribution

Three patches are ready for MariaDB upstream:
1. [BUG] CMake incorrectly detects `pthread_threadid_np()` on AIX
2. [BUG] CMake incorrectly detects `getthrid()` on AIX
3. [FEATURE] Add AIX pollset support for thread pool

See `SOURCES/` for patch files.

## License

- MariaDB: GPLv2 (MariaDB Foundation)
- AIX patches and packaging: GPLv2 (LibrePower)

## Credits

- MariaDB by [MariaDB Foundation](https://mariadb.org)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source

## Support

- **Repository**: https://aix.librepower.org
- **Source**: https://gitlab.com/librepower/aix
- **Newsletter**: https://librepower.substack.com
- **Email**: hello@librepower.org
