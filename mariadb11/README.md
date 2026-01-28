# MariaDB 11.8.5 LTS for AIX

> The first MariaDB for AIX with **native thread pool**. Up to **83% faster** concurrent workloads. Ready for **POWER9, POWER10, and POWER11**.

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.3+-green)](https://www.ibm.com/power/operating-systems/aix)
[![MariaDB](https://img.shields.io/badge/MariaDB-11.8.5_LTS-orange)](https://mariadb.org)
[![POWER](https://img.shields.io/badge/POWER-9_|_10_|_11-red)](https://www.ibm.com/power)
[![License](https://img.shields.io/badge/License-GPLv2-lightgrey)](https://www.gnu.org/licenses/gpl-2.0.html)

![MariaDB 11.8 on AIX Demo](demo.gif)

---

## Highlights

- **Native Thread Pool**: First MariaDB for AIX with `pool-of-threads` via `pollset(2)` -- up to 83% faster under concurrent load
- **Vector Search Ready**: MHNSW index support with optimized 64K large pages for POWER
- **Two Builds**: Open XL (fastest) and GCC (no dependencies) -- choose what fits your environment
- **POWER Optimized**: Built with `-O3 -mcpu=power9` for optimal performance on POWER9, POWER10, and POWER11
- **AIX Native**: SRC integration, 64K page support, AIX Toolbox compatible

---

## Performance

| Workload | Without Thread Pool | With Thread Pool | Improvement |
|----------|---------------------|------------------|-------------|
| Mixed 50 clients | 5.4s | 1.9s | **65% faster** |
| Mixed 100 clients | 11.3s | 2.0s | **83% faster** |

> Benchmarked on IBM Power S924 (POWER9), AIX 7.3 TL4

---

## Installation

### Option 1: DNF (Recommended)

```bash
curl -fsSL https://aix.librepower.org/install.sh | sh

# GCC build (no external dependencies)
dnf install mariadb11

# Open XL build (fastest, requires IBM Open XL runtime)
dnf install mariadb11-openxl
```

> **Note**: `mariadb11` and `mariadb11-openxl` conflict with each other -- only one can be installed at a time. Both install to the same paths (`/opt/freeware/mariadb`).

### Option 2: Direct RPM

**GCC Build** (no external dependencies)
```bash
curl -LO https://aix.librepower.org/packages/mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm
rpm -ivh mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm
```

**Open XL Build** (fastest performance)
```bash
curl -LO https://aix.librepower.org/packages/mariadb11-openxl-11.8.5-3.librepower.aix7.3.ppc.rpm
rpm -ivh mariadb11-openxl-11.8.5-3.librepower.aix7.3.ppc.rpm
```
> **Note**: The Open XL build requires IBM Open XL C/C++ 17.1.3 runtime. Please consult IBM for licensing requirements.

> Package names include `11` to coexist with AIX Toolbox's `mariadb10.11`.

---

## Quick Start

```bash
# Initialize database (first time only)
cd /opt/freeware/mariadb
mkdir -p /var/mariadb/data
chown mysql:staff /var/mariadb/data
./bin/mysql_install_db --basedir=/opt/freeware/mariadb --datadir=/var/mariadb/data --user=mysql

# Start
startsrc -s mariadb11

# Verify
/opt/freeware/mariadb/bin/mariadb -u root -e "SELECT VERSION();"
# -> 11.8.5-MariaDB
```

### SRC Management

```bash
startsrc -s mariadb11          # Start
stopsrc -s mariadb11           # Stop
lssrc -s mariadb11             # Status

# Auto-start on boot
mkitab "mariadb11:2:once:/usr/bin/startsrc -s mariadb11 > /dev/console 2>&1"
```

---

## Configuration

Default config at `/opt/freeware/mariadb/etc/mariadb11.cnf`:

```ini
[mariadbd]
thread_handling            = pool-of-threads
thread_pool_size           = 12
innodb_buffer_pool_size    = 1G
max_connections            = 2000
mhnsw_max_cache_size       = 4294967296   # 4GB for vector indexes
```

---

## What's Included

| Component | Description |
|-----------|-------------|
| **mariadbd** | Database server with thread pool |
| **mariadb** | Command-line client |
| **mariadb-dump** | Logical backup |
| **mariadb-admin** | Server administration |
| **Performance Schema** | Query analysis and monitoring |
| **SRC Integration** | Native AIX service management |

### Storage Engines

| Engine | Description |
|--------|-------------|
| **InnoDB** | ACID transactions, row-level locking (default) |
| **Aria** | Crash-safe MyISAM replacement |
| **MEMORY** | In-memory tables |
| **MyISAM** | Legacy storage engine |

---

## Build Information

| | Open XL Build | GCC Build |
|--|---------------|-----------|
| **Compiler** | IBM Open XL 17.1.3 | GCC 13.3.0 |
| **Optimization** | `-O3 -mcpu=power9` | `-O3 -mcpu=power9` |
| **RPM Size** | 37 MB | 39 MB |
| **Dependencies** | Open XL runtime | None |

### Compatibility

| Platform | Status |
|----------|--------|
| AIX 7.3+ | Supported |
| POWER9 | Optimized |
| POWER10 | Compatible |
| POWER11 | Compatible |

---

## Technical Details

This port adds native AIX support through 4 patches:

1. **Thread Pool**: Native `pollset(2)` backend with ONESHOT simulation
2. **Large Pages**: `MAP_ANON_64K` support for reduced TLB misses
3. **Performance Schema**: Platform detection fixes for AIX
4. **CMake**: Build system fixes for AIX

All patches are available in `SOURCES/` and will be submitted upstream once MariaDB provides feedback.

### Building from Source

```bash
git clone --branch mariadb-11.8.5 https://github.com/MariaDB/server.git
cd server && git submodule update --init

# Apply patches
patch -p1 < mariadb-aix-perfschema.patch
patch -p1 < threadpool_aix_pollset.patch
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch

# Configure and build
cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-mcpu=power9" -DCMAKE_CXX_FLAGS="-mcpu=power9"
gmake -j$(nproc)
```

---

## Package Contents

```
mariadb11/
+-- RPMS/
|   +-- mariadb11-openxl-11.8.5-3.librepower.aix7.3.ppc.rpm   (Open XL build)
|   +-- mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm           (GCC build)
+-- SPECS/
|   +-- mariadb11-openxl.spec
|   +-- mariadb11.spec
+-- SOURCES/
|   +-- threadpool_aix_pollset.patch
|   +-- mariadb-aix-perfschema.patch
|   +-- 0004-AIX-large-pages-MAP_ANON_64K.patch
+-- README.md
```

---

## License

- MariaDB: GPLv2 (MariaDB Foundation)
- AIX patches and packaging: GPLv3 (LibrePower)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **Source**: [gitlab.com/librepower/aix](https://gitlab.com/librepower/aix)
- **Newsletter**: [librepower.substack.com](https://librepower.substack.com)

---

<div align="center">

**Built by [LibrePower](https://librepower.org) -- Unlocking Power Systems**

[![Download](https://img.shields.io/badge/Download-aix.librepower.org-orange?style=for-the-badge)](https://aix.librepower.org)

</div>
