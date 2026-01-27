# MariaDB 11.8.5 LTS for AIX -- with Native Thread Pool

> The first MariaDB build for AIX with `pool-of-threads` support. Up to **83% faster** than one-thread-per-connection. Built, tested, and validated on POWER9.

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.3_TL4-green)](https://www.ibm.com/power/operating-systems/aix)
[![MariaDB](https://img.shields.io/badge/MariaDB-11.8.5_LTS-orange)](https://mariadb.org)
[![Thread Pool](https://img.shields.io/badge/Thread_Pool-pool--of--threads-brightgreen)]()
[![License](https://img.shields.io/badge/License-GPLv2-red)](https://www.gnu.org/licenses/gpl-2.0.html)

![MariaDB 11.8 on AIX Demo](demo.gif)

---

## Why This Matters

AIX has been a second-class citizen in the MariaDB ecosystem. No thread pool. No optimized builds. Limited to `one-thread-per-connection` -- a model that collapses under concurrent load.

**We fixed that.**

This port adds native AIX `pollset(2)` support to MariaDB's thread pool, bringing AIX to full parity with Linux, FreeBSD, Solaris, and Windows. The result speaks for itself:

| Workload | `one-thread-per-connection` | `pool-of-threads` | Improvement |
|----------|----------------------------|--------------------|-------------|
| Mixed 50 clients | 5.437s | 1.917s | **65% faster** |
| Mixed 100 clients | 11.340s | 1.964s | **83% faster** |
| Read 100 clients | 0.349s | 0.346s | Parity |

> Benchmarked with `mysqlslap` on AIX 7.3 TL4, IBM Power S924 (POWER9), GCC 13.3.0, `-O3 -mcpu=power9`.

### Release 2: MHNSW Vector Search Optimization

Release 2 adds tuning and large page support for MariaDB's MHNSW vector index on POWER:

- Default config with `mhnsw_max_cache_size = 4GB` (upstream default 16MB is too small for real workloads -- the graph cache has no LRU and evicts entirely when exceeded)
- Wrapper with `LDR_CNTRL` for 64K pages on all process segments (reduces TLB misses on POWER)
- Patch 4: `MAP_ANON_64K` support in `my_largepage.c` for `--large-pages` on AIX

---

## QA Validation

This isn't a proof-of-concept. It's production-tested:

| Test | Result |
|------|--------|
| **MTR Regression** | 709 pass, 50 fail (federated/platform -- 0 thread-pool failures) |
| **Stress** | Read/write/mixed, 10 to 200 clients -- all pass |
| **Extreme Concurrency** | 500 + 1,000 simultaneous clients -- all pass |
| **Sustained Load** | 30 minutes, 100 clients, 895 rounds, 11.15M queries -- **0 errors** |
| **Memory Stability** | 1,648,482,400 bytes across 29 checkpoints -- **zero drift** |
| **Data Integrity** | `CHECK TABLE` on all tables -- OK |

---

## Installation

### DNF (Recommended)

```bash
# Add LibrePower repository (one-time)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install mariadb11
```

### Direct RPM

```bash
curl -L -o mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm

rpm -ivh mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm
```

> Package named `mariadb11` to coexist with AIX Toolbox's `mariadb10.11`.

---

## Quick Start

```bash
# Initialize (first time only)
cd /opt/freeware/mariadb
mkdir -p /var/mariadb/data
chown mysql:mysql /var/mariadb/data
./bin/mysql_install_db --basedir=/opt/freeware/mariadb --datadir=/var/mariadb/data --user=mysql

# Start via AIX SRC
startsrc -s mariadb11

# Verify
lssrc -s mariadb11
/opt/freeware/mariadb/bin/mariadb -u root -S /tmp/mysql.sock -e "SELECT VERSION(); SHOW VARIABLES LIKE 'thread_handling';"
# -> 11.8.5-MariaDB
# -> pool-of-threads
```

### SRC Management

```bash
startsrc -s mariadb11          # Start
stopsrc -s mariadb11           # Stop
lssrc -s mariadb11             # Status

# Auto-start on boot
mkitab "mariadb11:2:once:/usr/bin/startsrc -s mariadb11 > /dev/console 2>&1"
```

### Verify Installation

```bash
lssrc -s mariadb11                         # SRC status
ps -ef | grep mariadbd                     # Process check
netstat -an | grep 3306                    # Port check
echo "SELECT VERSION();" | mariadb -u root -S /tmp/mysql.sock  # Connection test
```

### Recommended Production Configuration

```ini
[mariadbd]
thread_handling            = pool-of-threads
thread_pool_size           = 12
thread_stack               = 512K
innodb_buffer_pool_size    = 1G
innodb_adaptive_hash_index = ON
max_connections            = 2000

# Vector index (MHNSW) -- upstream default 16MB is too small.
# The graph cache has no LRU; it evicts entirely when exceeded.
# Set generously; memory is only used when vector indexes are accessed.
mhnsw_max_cache_size       = 4294967296
```

> `thread-stack=512K` is required with `-O3` builds -- aggressive inlining increases stack usage.
>
> A default config file is installed at `/opt/freeware/mariadb/etc/mariadb11.cnf` with these settings.

---

## Four Patches, One Goal: AIX Parity

Starting from upstream MariaDB, exactly **4 patches** bring full AIX support:

### Patch 1: `pthread_threadid_np` Detection (Bug Fix)
- **File**: `storage/perfschema/CMakeLists.txt`
- **Problem**: CMake falsely detects macOS-specific `pthread_threadid_np()` on AIX
- **Fix**: Platform guard -- `IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")`
- **Impact**: Prevents 80+ compilation errors in Performance Schema

### Patch 2: `getthrid` Detection (Bug Fix)
- **File**: `storage/perfschema/CMakeLists.txt`
- **Problem**: CMake falsely detects OpenBSD-specific `getthrid()` on AIX
- **Fix**: Platform guard -- `IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")`
- **Impact**: Prevents compilation errors in Performance Schema

### Patch 3: Thread Pool -- AIX pollset (Feature)
- **Files**: `sql/CMakeLists.txt`, `sql/threadpool_generic.h`, `sql/threadpool_generic.cc`
- **What**: Native `pollset(2)` backend for MariaDB's thread pool (191 lines)
- **Key innovations**:
  - **ONESHOT simulation**: Remove fds after events (`PS_DELETE`), re-add when ready (`PS_ADD`) -- simulates Linux `EPOLLONESHOT`
  - **Per-pollset mutex**: Serializes concurrent `pollset_poll_ext` between listener (blocking) and worker (non-blocking `trylock`) threads
- **Impact**: Enables `pool-of-threads` on AIX -- up to 83% faster

### Why the mutex?

MariaDB's thread pool has two concurrent `io_poll_wait` callers per group:
1. **Listener thread**: `io_poll_wait(timeout=-1)` -- blocking
2. **Worker thread**: `io_poll_wait(timeout=0)` -- non-blocking check before sleeping

Linux `epoll` + `EPOLLONESHOT` guarantees each fd fires once across concurrent waiters. AIX `pollset` has no such guarantee -- the same fd appears in both results, causing two threads to process the same connection (crash).

The per-pollset `pthread_mutex` with `trylock` for non-blocking callers solves this cleanly: workers skip if the listener is active, and `PS_DELETE` runs under the lock before any fd is returned.

### Patch 4: Large Pages -- AIX `MAP_ANON_64K` (Performance)
- **File**: `mysys/my_largepage.c`
- **What**: Adds AIX implementation of `--large-pages` using `MAP_ANON_64K` (0x400)
- **Key changes**:
  - Define `MAP_ANON_64K` on AIX if not already defined
  - AIX `my_get_large_page_sizes()` reporting 64K as available
  - `MAP_ANON_64K` flag in `my_large_malloc()` and `my_large_virtual_alloc()`
- **Impact**: All mmap regions use 64K pages with `--large-pages`, reducing TLB misses for memory-intensive workloads (MHNSW vector graph traversal)

**Status**: All four patches submitted to MariaDB upstream (JIRA).

---

## What's Included

- **mariadbd** -- Database server with pool-of-threads
- **mariadb** -- Command-line client
- **mariadb-admin** -- Server administration
- **mariadb-dump** -- Logical backup
- **mariadb-import** -- Data import
- **mariadb-slap** -- Load testing
- **mysql_install_db** -- Database initialization
- **mysqld_safe** -- Server startup wrapper
- **my_print_defaults** -- Configuration display
- **resolveip** -- IP/hostname resolver
- **perror** -- Error code lookup
- **Performance Schema** -- 1,247 performance instruments
- **SRC integration** -- Native AIX service management

### Storage Engines

| Engine | Support | Description |
|--------|---------|-------------|
| InnoDB | DEFAULT | Transactions, row-level locking, ACID |
| Aria | YES | Crash-safe tables with MyISAM heritage |
| MEMORY | YES | Hash based, stored in memory |
| MyISAM | YES | Traditional MySQL storage engine |

---

## Build Details

| Component | Details |
|-----------|---------|
| **Version** | MariaDB 11.8.5 LTS |
| **Release** | 2 (MHNSW performance fix + 64K pages) |
| **Platform** | AIX 7.3 TL4 |
| **Hardware** | IBM Power S924 (POWER9) |
| **Compiler** | GCC 13.3.0 |
| **CMake** | 4.2.0 |
| **Optimization** | `-O3 -mcpu=power9 -mtune=power9` |
| **Thread Handling** | pool-of-threads (pollset v11) |
| **Build Time** | ~15 minutes (`gmake -j96`) |
| **RPM Size** | 40 MB |
| **Patches** | 4 (2 CMake + 1 thread pool + 1 large pages) |

### Building from Source

```bash
# Clone MariaDB 11.8.5
git clone --branch mariadb-11.8.5 https://github.com/MariaDB/server.git
cd server && git submodule update --init

# Apply patches
patch -p1 < mariadb-aix-perfschema.patch
patch -p1 < threadpool_aix_pollset.patch
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch

# Configure
cmake . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-mcpu=power9 -mtune=power9" \
  -DCMAKE_CXX_FLAGS="-mcpu=power9 -mtune=power9" \
  -DCMAKE_INSTALL_PREFIX=/opt/freeware/mariadb \
  -DWITH_EMBEDDED_SERVER=OFF \
  -DPLUGIN_CONNECT=NO

# Build
gmake -j96
```

### Installation Paths

```
/opt/freeware/mariadb/bin/         # Server + client binaries
/opt/freeware/mariadb/lib/         # libserver.so (~60 MB)
/opt/freeware/mariadb/share/       # Error messages, SQL scripts
/var/mariadb/data/                 # Database files
```

---

## AIX-Specific Notes

### Threading Library

The RPM wrapper script automatically sets `LIBPATH` for pthread-enabled libstdc++. No manual configuration needed.

### Performance Schema

Fully working with 1,247 performance instruments. AIX-specific patches fix incorrect detection of macOS `pthread_threadid_np()` and OpenBSD `getthrid()`, falling back to POSIX `pthread_self()`.

---

## Package Contents

```
mariadb11/
+-- demo.gif                                          # Demo recording
+-- RPMS/
|   +-- mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm
+-- SPECS/
|   +-- mariadb11.spec
+-- SOURCES/
|   +-- mariadb-aix-perfschema.patch                  # CMake bug fixes (patches 1 & 2)
|   +-- threadpool_aix_pollset.patch                  # Thread pool implementation (patch 3)
|   +-- 0004-AIX-large-pages-MAP_ANON_64K.patch       # Large pages for AIX (patch 4)
+-- docs/
|   +-- PATCHES.md                                    # Technical analysis of patches
|   +-- BUGS_REPORT.md                                # Bug reports for upstream
|   +-- UPSTREAM_PR_GUIDE.md                          # Guide for submitting PRs
+-- README.md
```

---

## Known Limitations

- **Native AIO**: AIX uses simulated AIO (no `io_uring` or `libaio`). InnoDB compensates with 12+12 I/O threads. Native POSIX AIO or AIX IOCP is a future improvement.
- **CONNECT engine**: Disabled due to `libxml2` macro conflict. Not critical for most workloads.
- **MTR failures**: 50 of 759 tests fail -- all federated/platform-specific, none thread-pool related.

---

## Upstream Contribution

Four patches ready for MariaDB upstream:
1. **[BUG]** CMake incorrectly detects `pthread_threadid_np()` on AIX
2. **[BUG]** CMake incorrectly detects `getthrid()` on AIX
3. **[FEATURE]** Add AIX pollset support for thread pool
4. **[FEATURE]** Add AIX large page support (`MAP_ANON_64K`) in `my_largepage.c`

See `docs/UPSTREAM_PR_GUIDE.md` for the full submission process and `SOURCES/` for patch files.

---

## License

- MariaDB: GPLv2 (MariaDB Foundation)
- AIX patches and packaging: GPLv2 (LibrePower)

## Credits

- MariaDB by [MariaDB Foundation](https://mariadb.org)
- AIX port and packaging by [LibrePower](https://librepower.org)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **Source**: [gitlab.com/librepower/aix](https://gitlab.com/librepower/aix)
- **Newsletter**: [librepower.substack.com](https://librepower.substack.com)
- **Email**: [hello@librepower.org](mailto:hello@librepower.org)

---

<div align="center">

**Built by [LibrePower](https://librepower.org) -- Unlocking Power Systems through open source**

[![Download](https://img.shields.io/badge/Download-aix.librepower.org-orange?style=for-the-badge)](https://aix.librepower.org)
[![Newsletter](https://img.shields.io/badge/Subscribe-Newsletter-blue?style=for-the-badge)](https://librepower.substack.com)

</div>
